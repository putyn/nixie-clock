#include <webui.h>
#include <SPI.h>

#define LATCH_pin 16
#define OE_pin 2
#define HVEN_pin 15

uint16_t cie1931[11] = { 900, 890, 873, 844, 799, 734, 647, 533, 390, 213, 0};

void hw_setup() {

  //init SPI driver for '595
  SPI.begin();
  //latch pin for '595
  pinMode(LATCH_pin, OUTPUT);

  //output enable (low) for '595
  pinMode(OE_pin, OUTPUT);
  //PWM freq, any higher than this will cause HVPS to emit a high pitched sound
  analogWriteFreq(65);

  //HVPS enable
  pinMode(HVEN_pin, OUTPUT);
  digitalWrite(HVEN_pin, HIGH);
}

void hw_set_brightness(uint8_t brightness) {
  analogWrite(OE_pin, cie1931[brightness]);
}

void update_displays2() {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint8_t data[4] = {0};

  Serial.printf("%02d:%02d:%02d\n", local_time.hours, local_time.minutes, local_time.seconds);

  //prepare display data
  data[0] = local_time.hours / 10;
  data[1] = local_time.hours % 10;
  data[2] = local_time.minutes / 10;
  data[3] = local_time.minutes % 10;

  //send to '595
  send_display_data(data);
}
/*
 * to be called from a ticker instance, should run till setup is finished 
*/
void boot_animation() {
  uint8_t data[4] = {0};
  static uint8_t digit = 0;

  memset(data, digit++, 4);
  send_display_data(data);

  //reset digit
  if (digit > 9)
    digit = 0;
}
/*
 * boot_animation + ACP staff
 * stops after 15 seconds and sets next ACP
 */
void acp_animation() {
  
  //borrow boot_animation 
  boot_animation();

  if (settings.uptime - settings.next_acp >= 15) {
    //set brighness back to saved value;
    hw_set_brightness(settings.brightness);
    //deatch ticker
    acp.detach();
    //update display
    update_displays();
    /*
     * prepare next ACP substract 15 seconds so ACP will always start at *:*:30
     * acp_time set via webui in 5min increments, can't be off, default every hour
     */
    settings.next_acp = (settings.uptime - 15) + ((settings.acp_time == 0 ? 12 : settings.acp_time )* 300);
    settings.should_acp = true;

    //debug message
    Serial.printf("[ACP] finished ACP @ %02d:%02d:%02d\n", local_time.hours, local_time.minutes, local_time.seconds);
  }
}

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
        frames[frame_idx][digit] = value++;
        if (value > 9) {
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
    delay(75);
  }
}

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
