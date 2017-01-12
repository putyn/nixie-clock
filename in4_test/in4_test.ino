#include <SPI.h>

void update_displays(uint16_t num);

#define LATCH_pin 8
#define OE_pin 9



uint16_t num = 0;

void setup() {
  //init SPI driver for '595
  SPI.begin();
  //latch pin for '595
  pinMode(LATCH_pin, OUTPUT);
  //output enable (low) for '595
  pinMode(OE_pin, OUTPUT);
  digitalWrite(OE_pin, LOW);

  //leet
  update_displays(1337);
}

void loop() {

  update_displays(num++);
  if (num > 9999)
    num = 0;
  delay(50);
}

void update_displays(uint16_t number) {

  uint16_t data[4] = {0};

  data[0] = 1 << (number / 1000);
  data[1] = 1 << ((number / 100) % 10);
  data[2] = 1 << ((number / 10) % 10);
  data[3] = 1 << (number % 10);

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
