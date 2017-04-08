#include <SPI.h>

void update_displays(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4); 

#define LATCH_pin 16
#define OE_pin 2
#define HVEN_pin 15

//uint8_t hours = 12;
//uint8_t minutes = 0;
//uint8_t seconds = 0;

void setup() {
  //init SPI driver for '595
  Serial.begin(115200);
  SPI.begin();
  //latch pin for '595
  pinMode(LATCH_pin, OUTPUT);
  //output enable (low) for '595
  pinMode(OE_pin, OUTPUT);
  analogWrite(OE_pin, 256);
  //HVPS enable
  pinMode(HVEN_pin, OUTPUT);
  digitalWrite(HVEN_pin, HIGH);

  //test
  uint8_t number = 0;
  for(number; number < 100; number+=11){
    update_displays(number, number, number, number);  
    delay(1000);
  }
}

void loop() {

//  if(seconds++ > 59) {
//    seconds = 0;
//    minutes +=1;
//  }
//  if(minutes > 59) {
//    minutes = 0;
//    hours +=1;  
//  }
//  if(hours > 23) {
//    hours = 0;
//  }
//  char buffer[16] = {0};
//  sprintf(buffer,"%02d:%02d:%02d\n", hours,minutes,seconds);
//  Serial.print(buffer);
//  update_displays(hours, minutes);
//  delay(100);
}

void update_displays(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4) {

  int16_t data[4] = {0};

//  data[0] = 1 << (hours / 10);
//  data[1] = 1 << (hours % 10);
//  data[2] = 1 << (minutes / 10);
//  data[3] = 1 << (minutes % 10);

  //display 4
  SPI.transfer((uint8_t)(((1 << dig4) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << dig4) & 0x00FF));
  //display 3
  SPI.transfer((uint8_t)(((1 << dig3) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << dig3) & 0x00FF));
  //display 2
  SPI.transfer((uint8_t)(((1 << dig2) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << dig2) & 0x00FF));
  //display 1
  SPI.transfer((uint8_t)(((1 << dig1) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << dig1) & 0x00FF));

  //toggle latch pin
  digitalWrite(LATCH_pin, LOW);
  digitalWrite(LATCH_pin, HIGH);
  digitalWrite(LATCH_pin, LOW);
}
