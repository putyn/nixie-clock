#include <ESP8266Ping.h>
#include <Ticker.h>
#include <webui.h>

/*
 * settings from webui.h
 * device settings from webui.h
 */
extern settings_t settings;
extern device_t device;

/*
 * ticker for animation, clock
 * ticker for anti cathode poisoning
 */
Ticker tick;
Ticker  acp;

/*
 * time, is all about time
 * timer interrupt every 1 ms
 * main clock routine
 */
ctime_t local_time, old_time;

void tock(void) {
  
  //increase millis, save time for "slot" effect
  local_time.millis++;
  old_time = local_time;
  
  if (local_time.millis > 1000) {
    //uptime increase (used in webui)
	  device.uptime++;
    local_time.seconds++;
    local_time.millis = 0;

    //anti-cathode poisoning, first time
    if(local_time.seconds == 30 && device.acp_start_time == 0) {
      acp_animation();
    } 
  }
  //runs every minute
  if (local_time.seconds > 59) {
    local_time.seconds = 0;
    local_time.minutes += 1;

    //update display
    device.update_display = true;
  }
  //runs every hour
  if (local_time.minutes > 59) {
    local_time.minutes = 0;
    local_time.hours += 1;

    //check for change in night mode, set brightness, suppress ACP if needed
    if(device.night_mode != is_night()) {
      //set brigness to correct value
      hw_set_brightness(is_night() ? 0 : settings.brightness);

      //disable ACP if night mode and should suppress ACP
      if(is_night() && settings.suppress_acp) {
        acp.detach();
      }
      //it was night, it is no more, set acp_start_time to 0, it will be handled by the initial ACP start
      if(!is_night() && device.night_mode) {
        device.acp_start_time = 0;  
      }
      device.night_mode = is_night();
    }
  }
  //runs every 24 hours
  if (local_time.hours > 23) {
    local_time.hours = 0;
  }
}

void setup() {
  
  //for debug
  Serial.begin(74880);
  
  //file system, first for settings
  fs_setup();
  //hostname
  sprintf(device.hostname, "naked_nixie_%06X", ESP.getChipId());
  
  //brightness
  hw_set_brightness(is_night() ? 0 : settings.brightness);
  
  //hardware setup
  hw_setup();
  tick.attach_ms(250, boot_animation);

  //setup wifi
  wifi_setup();

  //setup web services
  web_setup();

  //check to see if time server is online (check internet connection also)
  device.online = Ping.ping(settings.time_server) > 0 ? true : false;
  
  if (device.online) {
    ntp_get_time(&local_time);
    old_time = local_time;
  }
  //deatach boot animation, attach clock;
  tick.attach_ms(1, tock);
  update_displays();

  Serial.printf("[SYS] night mode is %s\n", is_night() ? "on" : "off");
}
void loop() {
  //check reboot flag
  if (device.reboot) {
    Serial.println(F("[SYS] got restart request from the wire!"));
    device.reboot = false;
    ESP.restart();
  }
  //check soft_ap flag to handle DNS requests
  if (device.soft_ap) {
    webui_dns_requests();
  }
  //update display if necesary
  if (device.update_display) {
    device.update_display = false;
    update_displays();
  }
  //check update time flag or next update update
  if ((device.update_time || device.uptime > device.next_ntp_update) && device.online) {
    Serial.println(F("[NTP] time for an update!"));
    device.update_time = false;
    ntp_get_time(&local_time);
    update_displays();
  }
}
