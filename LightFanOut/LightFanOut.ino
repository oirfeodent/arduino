#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68

const   int     dcLightStartHour    = 10;
const   int     dcLightStartMins    = 58;
const   int     dcLightEndHour      = 11;
const   int     dcLightEndMins      = 2;
const   int     dcLightPin          = 7;
int     dcLightState        = LOW;
int     holdDcLightState    = LOW;

void readDS3231time(byte *second,
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year) {
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

byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

void printToSerial(byte *second,
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year1,
                    String *comment) {
  Serial.print(*hour, DEC);
  Serial.print(" ");
  Serial.print(*hour);
  Serial.print(":");
  if (*minute < 10)
  {
    Serial.print("0");
  }
  Serial.print(*minute, DEC);
  Serial.print(":");
  if (*second < 10)
  {
    Serial.print("0");
  }
  Serial.print(*second, DEC);
  Serial.print(" ");
  Serial.print(*dayOfMonth);
  Serial.print("/");
  Serial.print(*month, DEC);
  Serial.print("/");
  Serial.print(*year1, DEC);
  Serial.print(" -- ");
  Serial.println(*comment);
}

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  pinMode(dcLightPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  String comment;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
//  Serial.println(hour, DEC);
//  Serial.println(hour);
  if (hour >= dcLightStartHour && hour <= dcLightEndHour) {
    comment = "Inside Hours";
    printToSerial(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year, &comment );
    if (hour == dcLightStartHour && minute >= dcLightStartMins) {
      dcLightState = HIGH;
      comment = "After Mins";
      printToSerial(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year, &comment );
    } else if (hour == dcLightEndHour && minute <= dcLightEndMins) {
      dcLightState = HIGH;
      comment = "Before Mins";
      printToSerial(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year, &comment );
    } else if (hour > dcLightStartHour && hour < dcLightEndHour ) {
      dcLightState = HIGH;
      comment = "Other Hours";
      printToSerial(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year, &comment );
    } else {
      dcLightState = LOW;
      comment = "Outside Mins";
      printToSerial(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year, &comment );
    }
  } else {
    dcLightState = LOW;
    comment = "Outside Hours";
    printToSerial(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year, &comment );
  }
  if (holdDcLightState != dcLightState) {
    digitalWrite(dcLightPin, dcLightState);
    holdDcLightState = dcLightState;
  }
  Serial.println(dcLightState);
  delay(1000);
}
