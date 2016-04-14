#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68

const  int dcLightStartHour  = 8;
const  int dcLightStartMins = 5;
const  int dcLightEndHour = 16;
const  int dcLightEndMins = 55;
const  int dcLightPin = 7;
int dcLightState = LOW;
int holdDcLightState = LOW;

void readDS3231time(byte *second,
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

byte bcdToDec(byte val){
  return ( (val / 16 * 10) + (val % 16) );
}

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  pinMode(dcLightPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  if (hour >= dcLightStartHour && hour < dcLightEndHour) {
    if (hour = dcLightStartHour && minute < dcLightStartMins) {
      dcLightState = HIGH;
    } else if (hour = (dcLightEndHour - 1) && minute < dcLightStartMins) {
      dcLightState = HIGH;
    } else if (hour > dcLightStartHour && hour < (dcLightEndHour - 1)) {
      dcLightState = HIGH;
    } else {
      dcLightState = LOW;
    }
  } else {
    dcLightState = LOW;
  }
  if (holdDcLightState != dcLightState) {
    digitalWrite(dcLightPin, dcLightState);
    holdDcLightState = dcLightState;
  }
}
