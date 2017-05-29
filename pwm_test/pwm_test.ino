#include <SPI.h>
#include "cie1931.h"

#define LATCH_pin 16
#define OE_pin 2
#define HVEN_pin 15

String foo;
String boo;
uint8_t data[4];
uint8_t digit;
uint8_t brightness;

void setup() {

  //for debug resons
  Serial.begin(74880);

  //init SPI driver for '595
  SPI.begin();
  //latch pin for '595
  pinMode(LATCH_pin, OUTPUT);

  //output enable (low) for '595
  pinMode(OE_pin, OUTPUT);
  analogWriteFreq(60);
  analogWrite(OE_pin, cie1931[0]);

  //HVPS enable
  pinMode(HVEN_pin, OUTPUT);
  digitalWrite(HVEN_pin, HIGH);

  memset(data, 8, 4);
  send_display_data(data);
}

void loop() {
  if (Serial.available()) {
    foo = Serial.readStringUntil('\n');
    boo = foo.substring(1);

    switch (foo.charAt(0)) {
      case 'd' :
        Serial.printf("Setting digit to %d\n", boo.toInt());
        memset(data, boo.toInt(), 4);
        send_display_data(data);
        break;
      case 'b' :
        Serial.printf("Setting brightness to %d\n", boo.toInt());
        hw_set_brightness(boo.toInt());
        break;
      case 'f' :
        Serial.printf("Doing a brightness fade\n");
        for (brightness = 0; brightness < 11; brightness++) {
          hw_set_brightness(brightness);
          delay(250);
        }
        break;
      case 'r' :
        Serial.printf("Doing a digit roll\n");
        for (digit = 0; digit < 10; digit++) {
          memset(data, digit, 4);
          send_display_data(data);
          delay(250);
        }
        break;
      default:
        Serial.printf("Don't know what you want\n");
    }
  }
}

void hw_set_brightness(uint8_t brightness) {
  analogWrite(OE_pin, cie1931[brightness]);
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
