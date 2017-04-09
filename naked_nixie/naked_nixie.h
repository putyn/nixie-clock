#pragma once

//time structure
struct ctime_t {
  uint8_t mseconds;
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
} local_time, old_time;

//wifi settings structure
struct wifi_settings_t {
  char ssid[32];
  char pass[32];
} wifi;

//various other settings, some saved on flash
struct settings_t {
  char time_server[32];
  char hostname[32];
  int16_t time_zone;
  int16_t time_dst;
  uint32_t next_ntp_update;
  uint32_t uptime_seconds;
  boolean update_display;
  boolean update_time;
  boolean reboot;
  boolean online;
  boolean soft_ap;
} settings;

//process NTP requests
ctime_t ntp_get_time();
//send NTP requests
void ntp_send_request(IPAddress& address);
//hw interface
void update_displays(ctime_t t);
//time keeping
void tock();

//hardware defines
#define LATCH_PIN 16	//latch pin of 596
#define OE_PIN 2		//output enable pin of 596
#define HVPS_pin 15		//HVPS enable

//update NTP every 12h, in us
#define NTP_UPDATE 120 * 1000
