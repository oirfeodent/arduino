#include "Wire.h"
//#include <StepperAK.h> //use modified stepper library with 1000/0100/0010/0001 magnet firing sequence. Put library in your library folder.
#define DS3231_I2C_ADDRESS 0x68
//#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)          Serial.print (x)
  #define DEBUG_PRINTLN(x)        Serial.println (x)
  #define DEBUG_PRINTDEC(x)       Serial.print (x, DEC)
  #define DEBUG_SERIAL_BEGIN(x)   Serial.begin(9600)
  #define DEBUG_SLEEPTIMEINSEC()  const int sleepTimeInSec = 1
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_SERIAL_BEGIN(x)
  #define DEBUG_SLEEPTIMEINSEC()  const int sleepTimeInSec = 30
#endif

/*#define A1F 0
  #define RTC_STATUS 0x0F
  #define gearratio 64 //1:64 gear ratio
*/

const int startHour = 9;
const int endHour = 16;
const int fanStartHour = 8;
const int fanEndHour = 20;

const int lightPin =  7;
const int fanPin = 4;
const int acLightPin = 2;

DEBUG_SLEEPTIMEINSEC();

//const int stepsPerRevolution = 2048;  //the Arduino Kit motor is geared down. By experiment I determined that 2048 steps turn the shaft one round.

int lightState = LOW;
int acLightState = LOW;
int fanState = LOW;

byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

// instantiate a 4-wire stepper on pins 8 through 11:
//Stepper myStepper(stepsPerRevolution, 8,9,10,11);

void setup()
{
  Wire.begin();
  DEBUG_SERIAL_BEGIN(9600);

  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, lightState);

  pinMode(acLightPin, OUTPUT);
  digitalWrite(acLightPin, acLightState);

  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, fanState);

  // set the speed at 0.15 rpm (0.15-0.2 (dep. on particular motor) seems the fastest they can go...set it higher and it stops moving, just makes noises):
  //  myStepper.setSpeed(0.15*gearratio);//the motor appears to be geared down 1/64, meaning that the speed needs to be set 64x.

}
void readDS3231time(byte *second,
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year)
{
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

void loop()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  if (hour >= startHour && hour < endHour) {
    // send it to the serial monitor
    DEBUG_PRINTDEC(hour);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(hour);
    // convert the byte variable to a decimal number when displayed
    DEBUG_PRINT(":");
    if (minute < 10)
    {
      DEBUG_PRINT("0");
    }
    DEBUG_PRINTDEC(minute);
    DEBUG_PRINT(":");
    if (second < 10)
    {
      DEBUG_PRINT("0");
    }
    DEBUG_PRINTDEC(second);
    DEBUG_PRINT(" ");
    DEBUG_PRINTDEC(dayOfMonth);
    DEBUG_PRINT("/");
    DEBUG_PRINTDEC(month);
    DEBUG_PRINT("/");
    DEBUG_PRINTDEC(year);
    DEBUG_PRINTLN(" Time Between");
    if (lightState == HIGH) {
      lightState = LOW;
      digitalWrite(lightPin, lightState);
    }
    if (acLightState == HIGH) {
      acLightState = LOW;
      digitalWrite(acLightPin, acLightState);
    }
  } else {
    // send it to the serial monitor
    DEBUG_PRINTDEC(hour);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(hour);
    // convert the byte variable to a decimal number when displayed
    DEBUG_PRINT(":");
    if (minute < 10)
    {
      DEBUG_PRINT("0");
    }
    DEBUG_PRINTDEC(minute);
    DEBUG_PRINT(":");
    if (second < 10)
    {
      DEBUG_PRINT("0");
    }
    DEBUG_PRINTDEC(second);
    DEBUG_PRINT(" ");
    DEBUG_PRINTDEC(dayOfMonth);
    DEBUG_PRINT("/");
    DEBUG_PRINTDEC(month);
    DEBUG_PRINT("/");
    DEBUG_PRINTDEC(year);
    DEBUG_PRINTLN(" TIME Outside");
    if (lightState == LOW) {
      lightState = HIGH;
      digitalWrite(lightPin, lightState);
    }
    if (acLightState == LOW) {
      acLightState = HIGH;
      digitalWrite(acLightPin, acLightState);
    }
  }
  if (hour >= fanStartHour && hour < fanEndHour) {
    if (fanState == HIGH) {
      fanState = LOW;
      digitalWrite(fanPin, fanState);
    }
  } else {
    if (fanState == LOW) {
      fanState = HIGH;
      digitalWrite(fanPin, fanState);
    }
  }
  delay(5000);
  /*
    if (alarmSet(1) == true)
    {
        DEBUG_PRINTLN(" Alarm 1 Triggered ");
        myStepper.step(stepsPerRevolution);
    }

    if (alarmSet(2) == true)
    {
        DEBUG_PRINTLN(" Alarm 2 Triggered ");
        myStepper.step(-stepsPerRevolution);
    }
  */
  //  delay(sleepTimeInSec * 1000);

}

/*
  boolean alarmSet(byte alarmNumber)
  {
    uint8_t statusReg, mask;

    statusReg = readRTC(RTC_STATUS);
    mask = _BV(A1F) << (alarmNumber - 1);
    if (statusReg & mask) {
        statusReg &= ~mask;
        writeRTC(RTC_STATUS, statusReg);
        return true;
    }
    else {
        return false;
    }
  }

  byte readRTC(byte addr)
  {
    byte b;

    //readRTC(addr, &b, 1);

    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    Wire.write(addr);
    if ( byte e = Wire.endTransmission() ) return e;
    Wire.requestFrom( (uint8_t)DS3231_I2C_ADDRESS, (uint8_t)1 );
    b = Wire.read();
    return b;
  }

  byte writeRTC(byte addr, byte value)
  {
    //return ( writeRTC(addr, &value, 1) );
    Wire.beginTransmission(DS3231_I2C_ADDRESS );
    Wire.write(addr);
    Wire.write(value);
    return Wire.endTransmission();
  }
*/
