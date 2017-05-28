#include <ESP8266Ping.h>
#include <Ticker.h>
#include <webui.h>

/*
 * settings define inside webui.h
 */
extern settings_t settings;

/*
 * ticker for animation
 */
Ticker animation;

/*
 * time, is all about time
 * timer interrupt every 1 ms
 * main clock routine
 */
ctime_t local_time;

void timer_callback(void) {

  local_time.millis++;
  if (local_time.millis > 1000) {
	settings.uptime++;
    local_time.seconds++;
    local_time.millis = 0;
    settings.update_display = true;
  }
  if (local_time.seconds > 59) {
    local_time.seconds = 0;
    local_time.minutes += 1;
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
  timer1_write(5002);
  timer1_isr_init();

  if (settings.online) {
    ntp_get_time(&local_time);
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
    settings.update_display = false;
    update_displays();
  }
  //check update time flag or next update update
  if ((settings.update_time || millis() > settings.next_ntp_update) && settings.online) {
    Serial.println(F("[NTP] time for an update!"));
    settings.update_time = false;
    ntp_get_time(&local_time);
  }
}


