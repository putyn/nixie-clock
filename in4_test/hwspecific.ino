#include <webui.h>
#include <SPI.h>

#define LATCH_pin 16
#define OE_pin 2
#define HVEN_pin 15


void hw_setup() {
  
  //for debug resons
  Serial.begin(74880);
  
  //init SPI driver for '595
  SPI.begin();
  //latch pin for '595
  pinMode(LATCH_pin, OUTPUT);
  
  //output enable (low) for '595
  pinMode(OE_pin, OUTPUT);
  digitalWrite(OE_pin, LOW);
  
  //HVPS enable
  pinMode(HVEN_pin, OUTPUT);
  digitalWrite(HVEN_pin, HIGH);
}


void update_displays() {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds; 
  uint8_t data[4] = {0};
  /*
  hours = (unix_time  % 86400L) / 3600;
  minutes = (unix_time  % 3600) / 60;
  seconds = unix_time % 60;
  */
  Serial.printf("%02d:%02d:%02d\n", local_time.hours,local_time.minutes,local_time.seconds);

  //prepare display data
  data[0] = local_time.hours / 10;
  data[1] = local_time.hours % 10;
  data[2] = local_time.minutes / 10;
  data[3] = local_time.minutes % 10;

  //send to '595
  send_display_data(data);
}
/*
 * to be called from a instance ticker, should run till setup is finished
 */
void boot_animation() {
  uint8_t data[4] = {0};
  static uint8_t digit = 0;
  
  memset(data,digit++,4);
  send_display_data(data);
  
  //reset digit
  if(digit > 9)
    digit = 0; 
}

/*
void update_displays() {
  
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
    send_display_data(frames[frame_idx]);
    delay(100);
  }
}
*/
void send_display_data(uint8_t *data) {

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
  digitalWrite(LATCH_pin, LOW);
  digitalWrite(LATCH_pin, HIGH);
  digitalWrite(LATCH_pin, LOW);
}
