#include <ESP8266Ping.h>
#include <Ticker.h>
#include <webui.h>

/*
 * settings from webui.h
 */
extern settings_t settings;

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
	  settings.uptime++;
    local_time.seconds++;
    local_time.millis = 0;

    //anti-cathode poisoning, first time
    if(local_time.seconds == 29 && settings.next_acp == 0) {
      settings.next_acp = settings.uptime + 1;
      settings.should_acp = true;
    } 
  }
  
  if (local_time.seconds > 59) {
    local_time.seconds = 0;
    local_time.minutes += 1;

    //update display
    settings.update_display = true;
  }
  
  if (local_time.minutes > 59) {
    local_time.minutes = 0;
    local_time.hours += 1;
  }
  
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
  sprintf(settings.hostname, "naked_nixie_%06X", ESP.getChipId());
  //brightness
  hw_set_brightness(settings.brightness);
  
  //hardware setup
  hw_setup();
  tick.attach_ms(250, boot_animation);

  //setup wifi
  wifi_setup();

  //setup web services
  web_setup();

  //check to see if time server is online (check internet connection also)
  settings.online = Ping.ping(settings.time_server) > 0 ? true : false;
  
  if (settings.online) {
    ntp_get_time(&local_time);
    old_time = local_time;
  }
  //deatach boot animation, attach clock;
  tick.attach_ms(1, tock);
  update_displays();
  
}

void loop() {
  //check reboot flag
  if (settings.reboot) {
    Serial.println(F("[SYS] got restart request from the wire!"));
    settings.reboot = false;
    ESP.restart();
  }
  //check soft_ap flag to handle DNS requests
  if (settings.soft_ap) {
    webui_dns_requests();
  }
  //update display if necesary
  if (settings.update_display) {
    settings.update_display = false;
    update_displays();
  }
  //check update time flag or next update update
  if ((settings.update_time || settings.uptime > settings.next_ntp_update) && settings.online) {
    Serial.println(F("[NTP] time for an update!"));
    settings.update_time = false;
    ntp_get_time(&local_time);
    update_displays();
  }
  //check for acp
  if(settings.uptime >= settings.next_acp && settings.should_acp) {
    settings.should_acp = false;
    hw_set_brightness(10);
    acp.attach_ms(250,acp_animation);
    Serial.printf("[ACP] started ACP @ %02d:%02d:%02d\n", local_time.hours, local_time.minutes, local_time.seconds);
  }
}
