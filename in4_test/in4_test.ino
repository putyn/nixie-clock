#include <SPI.h>

void update_displays(uint8_t hours, uint8_t minutes);

#define LATCH_pin 8
#define OE_pin 9

uint8_t hours = 12;
uint8_t minutes = 0;
uint8_t seconds = 0;

void setup() {
  //init SPI driver for '595
  Serial.begin(115200);
  SPI.begin();
  //latch pin for '595
  pinMode(LATCH_pin, OUTPUT);
  //output enable (low) for '595
  pinMode(OE_pin, OUTPUT);
  digitalWrite(OE_pin, LOW);

  //leet
  update_displays(13,37);
  delay(2000);
}

void loop() {

  if(seconds++ > 59) {
    seconds = 0;
    minutes +=1;
  }
  if(minutes > 59) {
    minutes = 0;
    hours +=1;  
  }
  if(hours > 23) {
    hours = 0;
  }
  char buffer[16] = {0};
  sprintf(buffer,"%02d:%02d:%02d\n", hours,minutes,seconds);
  Serial.print(buffer);
  update_displays(hours, minutes);
  delay(100);
}

void update_displays(uint8_t hours, uint8_t minutes) {

  int16_t data[4] = {0};

  data[0] = 1 << (hours / 10);
  data[1] = 1 << (hours % 10);
  data[2] = 1 << (minutes / 10);
  data[3] = 1 << (minutes % 10);

  //display 4
  SPI.transfer((uint8_t)((data[3] & 0xFF00) >> 8));
  SPI.transfer((uint8_t)(data[3] & 0x00FF));
  //display 3
  SPI.transfer((uint8_t)((data[2] & 0xFF00) >> 8));
  SPI.transfer((uint8_t)(data[2] & 0x00FF));
  //display 2
  SPI.transfer((uint8_t)((data[1] & 0xFF00) >> 8));
  SPI.transfer((uint8_t)(data[1] & 0x00FF));
  //display 1
  SPI.transfer((uint8_t)((data[0] & 0xFF00) >> 8));
  SPI.transfer((uint8_t)(data[0] & 0x00FF));

  //toggle latch pin
  digitalWrite(LATCH_pin, LOW);
  digitalWrite(LATCH_pin, HIGH);
  digitalWrite(LATCH_pin, LOW);
}
