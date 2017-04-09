/**
 * basic clock in software, using system ticker to cound seconds
*/
void tock() {
  /**
   * clock part
   * increase seconds every tick
  */
  old_time = local_time;
  local_time.seconds += 1;
  if (local_time.seconds > 59) {
    local_time.seconds = 0;
    local_time.minutes += 1;
    settings.update_display = 1;
  }
  if (local_time.minutes > 59) {
    local_time.minutes = 0;
    local_time.hours += 1;
  }
  if (local_time.hours > 23) {
    local_time.hours = 0;
  } 
  /**
   * since this is an interrupt we dont want to execute too much code here
   * we will update the clock in the loop
  */
  settings.update_display = 1;
}
void boot_animation() {
  uint8_t data[4] = {0};
  uint8_t digit;
  for(digit = 0; digit < 10; digit++) {
    data[0] = data[1] = data[2] = data[3] = digit;
    update_displays(data);
    delay(250);
  }
  data[0] = local_time.hours / 10;
  data[1] = local_time.hours % 10;
  data[2] = local_time.minutes / 10;
  data[3] = local_time.minutes % 10;
  update_displays(data);
}

void slot() {
  
  uint8_t old_values[4] = {0};
  uint8_t new_values[4] = {0};

  //break new/old time into individual digits
  //old time
  old_values[0] = (old_time.hours / 10);
  old_values[1] = (old_time.hours % 10);
  old_values[2] = (old_time.minutes / 10);
  old_values[3] = (old_time.minutes % 10);
  
  //new time
  new_values[0] = (local_time.hours / 10);
  new_values[1] = (local_time.hours % 10);
  new_values[2] = (local_time.minutes / 10);
  new_values[3] = (local_time.minutes % 10);
  
  uint8_t frames[11][4] = {0};
  uint8_t frame_idx = 0;
  uint8_t digit;
  uint8_t value;

  for (digit = 0; digit < 4; digit++) {
    if (new_values[digit] != old_values[digit] ) {
      
      value = new_values[digit];
      for (frame_idx = 0; frame_idx < 11; frame_idx++) {
        frames[frame_idx][digit]=value++;
        if(value > 9) {
          value = 0;
        }
      }
    } else {
      for (frame_idx = 0; frame_idx < 11; frame_idx++) {
        frames[frame_idx][digit] = old_values[digit];
      }
    }
  }

  for (frame_idx = 0; frame_idx < 11; frame_idx++) {
    //Serial.printf("Frame [%2d]: value [%2d:%2d:%2d:%2d]\n", frame_idx, frames[frame_idx][0], frames[frame_idx][1], frames[frame_idx][2], frames[frame_idx][3]);
    update_displays(frames[frame_idx]);
    delay(100);
  }
}

void update_displays(uint8_t *data) {

  //display 4
  SPI.transfer((uint8_t)(((1 << data[3]) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << data[3]) & 0x00FF));
  //display 3
  SPI.transfer((uint8_t)(((1 << data[2]) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << data[2]) & 0x00FF));
  //display 2
  SPI.transfer((uint8_t)(((1 << data[1]) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << data[1]) & 0x00FF));
  //display 1
  SPI.transfer((uint8_t)(((1 << data[0]) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << data[0]) & 0x00FF));

  //toggle latch pin
  digitalWrite(LATCH_PIN, LOW);
  digitalWrite(LATCH_PIN, HIGH);
  digitalWrite(LATCH_PIN, LOW);
}
/*
 * return size of file human readable
 */
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}
/*
 * rssi to quality for sorting networks
 * based on http://www.speedguide.net/faq/how-does-rssi-dbm-relate-to-signal-quality-percent-439
 */
int rssi2quality(int rssi) {
  int quality;

  if ( rssi <= -100) {
    quality = 0;
  } else if ( rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * ( rssi + 100);
  }
  return quality;
}
/*
 * helper function for saving data structures to file
 * havely based on  https://github.com/letscontrolit/ESPEasy/blob/mega/src/Misc.ino#L767
 */
boolean save_file(char* fname, byte* memAddress, int datasize){

  fs::File f = SPIFFS.open(fname, "w+");
  if (f) {
    byte *pointerToByteToSave = memAddress;
    for (int x = 0; x < datasize ; x++) {
      f.write(*pointerToByteToSave);
      pointerToByteToSave++;
    }
    f.close();
  }
  return true;
}
/*
 * helper function for reading data structures from file
 * havely based on  https://github.com/letscontrolit/ESPEasy/blob/mega/src/Misc.ino#L801
 */
void read_file(char* fname, byte* memAddress, int datasize) {
  fs::File f = SPIFFS.open(fname, "r+");
  if (f) {
    byte *pointerToByteToRead = memAddress;
    for (int x = 0; x < datasize; x++) {
      *pointerToByteToRead = f.read();
      pointerToByteToRead++;// next byte
    }
    f.close();
  }
}
/*
 * pretty uptime
 
String mkuptime(uint32_t seconds) {
  uint8_t days;
  uint8_t hours;
  uint8_t minutes;

  days = seconds / 86400;
  seconds = seconds % days;
  hours = seconds / 3600;
  seconds = seconds % hours;
  minutes = seconds / 60;
  seconds = seconds % minutes;

  char buf[32] = {0};
  sprintf(buf, "%d days, %d hours, %d minutes, %d seconds", days, hours, minutes, seconds);
  Serial.println(buf);
}
*/
