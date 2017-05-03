#include <ESP8266Ping.h>
#include <Ticker.h>
#include <webui.h>
extern settings_t settings;

Ticker animation;


uint32_t unix_time = 0;
uint32_t unix_time_ms = 0;

void timer_callback(void) {
  unix_time_ms++;
  if(unix_time_ms > 1000) {
    unix_time++;
    unix_time_ms = 0; 
    settings.update_display = 1; 
  }
}

void setup() {

  hw_setup();
  animation.attach_ms(250, boot_animation);

  //setup file system
  fs_setup();
  //set a nice hostname
  sprintf(settings.hostname, "naked_nixie_%06X", ESP.getChipId());

  //setup wifi
  wifi_setup();

  //setup web services
  web_setup();

  //check to see if time server is online (check internet connection also)
  settings.online = Ping.ping(settings.time_server) > 0 ? true : false;
  
  //enable timer1 as time base with interrupt every 1ms
  timer1_disable();
  timer1_attachInterrupt(timer_callback);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
  timer1_write(5000);
  timer1_isr_init();

  if(settings.online) {
    unix_time = ntp_get_time();
    update_displays();  
  }
  //deatach bootanimation;
  animation.detach();
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
    settings.update_display = 0;
    update_displays();
  }
}


