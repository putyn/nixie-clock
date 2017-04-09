/*
 * callback for /overview GET
 * returns system information
 * json format
 * based on https://bitbucket.org/xoseperez/espurna/src/6969ee84a09827b0d146c09b08ad5449357d13c6/code/espurna/espurna.ino?at=master&fileviewer=file-view-default#espurna.ino-115
 */
void handle_overview(AsyncWebServerRequest *request) {

  String json_resp;
  char chipid[6] = {0};
  sprintf(chipid, "%06X", ESP.getChipId());
   
  json_resp = "{\"ChipID\": \"" + String(chipid) + "\",";
  json_resp += "\"CPU frequency\": \"" + String(ESP.getCpuFreqMHz()) + " MHz\",";
  json_resp += "\"Last reset reason\": \"" + ESP.getResetReason() + "\",";
  json_resp += "\"Internet\": \"" + String(settings.online ? "online" : "offline") + "\",";
  json_resp += "\"Uptime\": \"" + String(millis()/1000) + "s\",";
  json_resp += "\"Free heap\": \"" + formatBytes(ESP.getFreeHeap()) + "\",";
  json_resp += "\"Storage size\": \"" + formatBytes(ESP.getFlashChipSize()) + "\",";
  json_resp += "\"Firmware size\": \"" + formatBytes(ESP.getSketchSize()) + "\",";
  json_resp += "\"Free firmware space\": \"" + formatBytes(ESP.getFreeSketchSpace()) + "\"}";
  
  request->send(200, "text/json", json_resp);
}
/*
 * callback for /wifi POST
 * saves wifi settings to file
 * json format
 */
void handle_wifi_save(AsyncWebServerRequest *request) {
  //json response
  String json_resp;
  
  if (request->hasParam("ssid", true) && request->hasParam("pass", true)) {
    
    //get ssid and pass from POST request
    strncpy(wifi.ssid, request->getParam("ssid", true)->value().c_str(), sizeof (wifi.ssid));
    strncpy(wifi.pass, request->getParam("pass", true)->value().c_str(), sizeof (wifi.pass));

    if (save_file((char *)"/wifi.dat", (byte *)&wifi, sizeof(struct wifi_settings_t))) {
      json_resp = F("{\"error\":false, \"message\": \"Settings saved, reboot to load new network config\"}");
    } else {
      json_resp = F("{\"error\":true, \"message\": \"Settings could not be saved\"}");
    }
  } else {
    json_resp = F("{\"error\":true, \"message\": \"Bad request\"}");
  }
  request->send(200, "text/json", json_resp);
}
/*
 * callback for /wifi GET :
 * returns network status and available network list
 * json format
 */
void handle_wifi(AsyncWebServerRequest *request) {

  uint8_t found_networks = 0;
  uint8_t idx = 0;
  String json_resp;

  if ((WiFi.getMode() & WIFI_AP_STA) == WIFI_AP_STA) {
    json_resp = "{\"status\":{\"Mode\":\"Access Point\",\"IP\":\"" + WiFi.softAPIP().toString() + "\",\"Hostname\":\"" + WiFi.hostname() + "\"},\"networks\":[";
  } else {
    json_resp = "{\"status\":{\"Mode\":\"Client\",\"SSID\":\"" + WiFi.SSID() + "\",\"IP\":\"" + WiFi.localIP().toString() + "\",\"Hostname\":\"" + WiFi.hostname() + "\"},\"networks\":[";
  }

  found_networks = WiFi.scanComplete();
  if ( found_networks > 0) {
    for (idx = 0; idx < found_networks; ++idx) {
      if (idx)
        json_resp += ",";
      json_resp += "{\"ssid\":\"" + WiFi.SSID(idx) + "\",\"auth\":" + (WiFi.encryptionType(idx) == ENC_TYPE_NONE ? 0 : 1) + ",\"quality\":" + rssi2quality(WiFi.RSSI(idx)) + "}";
    }
    WiFi.scanDelete();
    WiFi.scanNetworks(1);
  }
  json_resp += "]}";
  request->send(200, "text/json", json_resp);
}
/*
 * callback for /time GET
 * returns saved time settings
 * json format
 */
void handle_time(AsyncWebServerRequest *request) {
  
  String json_resp = "";

  json_resp = "{\"time_server\":\""+ String(settings.time_server) +"\",";
  if(settings.time_zone > 0) {
    json_resp += "\"time_zone\":\"+" + String(settings.time_zone) + "\",";  
  } else {
    json_resp += "\"time_zone\":" + String(settings.time_zone) + ",";
  }
  json_resp +=  "\"time_dst\": "+ String(settings.time_dst) +"}";
  request->send(200, "text/json", json_resp);
}
/*
 * callback for /time POST
 * saves time settings from server
 * returns messages based on data submited
 * json format
 */
void handle_time_save(AsyncWebServerRequest *request) {
  //json response
  String json_resp;
  
  if (request->hasParam("time_server", true) && request->hasParam("time_zone", true) && request->hasParam("time_dst", true)) {
    
    //get time_server, time_zone, time_dst from POST request
    strncpy(settings.time_server, request->getParam("time_server", true)->value().c_str(), sizeof (settings.time_server));
    settings.time_zone = request->getParam("time_zone", true)->value().toInt();
    settings.time_dst  = request->getParam("time_dst", true)->value().toInt();
    
    //update time with new settings
    Serial.printf("Time server: %s\n", settings.time_server);
    settings.online = Ping.ping(settings.time_server) > 0 ? true : false;
    Serial.println(Ping.ping(settings.time_server));
    settings.update_time = true;
    
    if (save_file((char *)"/settings.dat", (byte *)&settings, sizeof(struct settings_t))) {
      json_resp = F("{\"error\":false, \"message\": \"\"}");
    } else {
      json_resp = F("{\"error\":true, \"message\": \"settings could not be saved\"}");
    }
  } else {
    json_resp = F("{\"error\":true, \"message\": \"bad request\"}");
  }
  
  request->send(200, "text/json", json_resp);
}
