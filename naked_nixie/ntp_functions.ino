//ntp client
WiFiUDP ntp;
#define NTP_PACKET_SIZE 48
/**
 * get time via NTP
 * returns time in the custom structure
 * havely based on https://www.arduino.cc/en/Tutorial/UdpNTPClient
 */
ctime_t ntp_get_time() {
  
  //local port for receving data back
  static uint16_t local_port = 8266;
  
  //start udp client
  ntp.begin(local_port);
  
  //debug info
  Serial.printf("[NTP] UDP client on port: %d\n", ntp.localPort());
  
  //70 years worth of seconds
  static uint32_t seventy_years = 2208988800UL;
  
  //ntp ip
  IPAddress ntp_server_ip;
  
  //ntp buffer
  uint8_t ntp_packet[NTP_PACKET_SIZE];
  
  //unix time in seconds starting in 1900
  uint32_t sec_since1900 = 0;
  
  //unitx time from 1970
  uint32_t epoch = 0;
  
  ctime_t tmp_time;
  
  //get ip from the pool
  if(!WiFi.hostByName(settings.time_server, ntp_server_ip)) {
    Serial.printf("[NTP] can't get ip for host %s\n", settings.time_server);
    return local_time;  
  }
  
  //debug info
  Serial.printf("[NTP] server IP from pool: %s\n",ntp_server_ip.toString().c_str());
  
  //send the request
  ntp_send_request(ntp_server_ip);
  //debug info
  Serial.println(F("[NTP] requested time, waitting for response"));

  //wait until we get a packet with timeout
  uint32_t ntp_time_out = millis();
  boolean ntp_got_packet = false;
  while (!(millis() > ntp_time_out + 5000)) {
    if(ntp.parsePacket() == NTP_PACKET_SIZE) {
      Serial.printf("[NTP] got message in %d\n",millis() - ntp_time_out);
      ntp_got_packet = true;
      break;
    }
  }

  if(!ntp_got_packet) {
    Serial.println(F("[NTP] didn't get a message from the server, closing connection"));
    //next update in 5 minutes
    settings.next_ntp_update = millis() + 60000;
    ntp.stop();
    return local_time;
  }
  
  //debug info
  Serial.println(F("[NTP] got NTP packet, setting the time"));
  
  //read the packet
  ntp.read(ntp_packet, NTP_PACKET_SIZE);

  //seconds since 1900 unix time
  sec_since1900 = (word(ntp_packet[40], ntp_packet[41])) << 16 | word(ntp_packet[42], ntp_packet[43]);
  
  //seconds since 1970 unix time + time zone +/- dst
  epoch = sec_since1900 - seventy_years + settings.time_zone + settings.time_dst;
  
  // hour, minute and second:
  tmp_time.hours = (epoch  % 86400L) / 3600;
  tmp_time.minutes = (epoch  % 3600) / 60;
  tmp_time.seconds = epoch  % 60;

  //set next ntp update
  settings.next_ntp_update = millis() + NTP_UPDATE;
  ntp.stop();
  
  return tmp_time;
}
/**
 * sends the requesto to the specified NTP server
 * @param npt_server_ip IPAddress
 */
void ntp_send_request(IPAddress& ntp_server_ip) {

  uint8_t ntp_packet[NTP_PACKET_SIZE];
  Serial.println(F("[NTP] requesting NTP packet"));
  // set all bytes in the buffer to 0
  memset(ntp_packet, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  ntp_packet[0] = 0b11100011;   // LI, Version, Mode
  ntp_packet[1] = 0;     // Stratum, or type of clock
  ntp_packet[2] = 6;     // Polling Interval
  ntp_packet[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  ntp_packet[12]  = 49;
  ntp_packet[13]  = 0x4E;
  ntp_packet[14]  = 49;
  ntp_packet[15]  = 52;

  /**
   *all NTP fields have been given values, now
   *you can send a packet requesting a timestamp
   *NTP requests are to port 123
   */
  ntp.beginPacket(ntp_server_ip, 123);
  ntp.write(ntp_packet, NTP_PACKET_SIZE);
  ntp.endPacket();
}
