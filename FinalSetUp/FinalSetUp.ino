#include <Wire.h>
#include <OneWire.h>

#define DS3231_I2C_ADDRESS 0x68
//#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)          Serial.print (x)
  #define DEBUG_PRINTLN(x)        Serial.println (x)
  #define DEBUG_SERIAL_BEGIN(x)   Serial.begin(x)
  #define DEBUG_PRINTDEC(x)       Serial.print (x, DEC)
  #define DEBUG_SLEEPTIMEINSEC()  const int sleepTimeInSec = 1
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_SERIAL_BEGIN(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_SLEEPTIMEINSEC()  const int sleepTimeInSec = 30
#endif

DEBUG_SLEEPTIMEINSEC();

//All PINs to know available pins
const   int     acLightPin            = 2;
const   int     dcLightPin            = 3;
const   int     dcFanPin              = 4;
const   int     dcFanArduinoPin       = 5;
OneWire ds(6);  // on pin 6

byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

const   int     acLightStartHour      = 7;
const   int     acLightStartMins      = 30;
const   int     acLightEndHour        = 14;
const   int     acLightEndMins        = 30;
        int     acLightState          = HIGH;
        int     holdAcLightState      = HIGH;

const   int     dcLightStartHour      = 7;
const   int     dcLightStartMins      = 45;
const   int     dcLightEndHour        = 14;
const   int     dcLightEndMins        = 15;
        int     dcLightState          = HIGH;
        int     holdDcLightState      = HIGH;

        int     dcFanState            = HIGH;
        int     holdDcFanState        = HIGH;
const   float   setTemperature        = 28.0;
        float   temperature;

        int     dcFanArduinoState     = HIGH;
        int     holdDcFanArduinoState = HIGH;

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

// Read Time from DS3231
void readDS3231time()
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  second = bcdToDec(Wire.read() & 0x7f);
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read() & 0x3f);
  dayOfWeek = bcdToDec(Wire.read());
  dayOfMonth = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
}

// Display Time Read from DS3231
void displayTime()
{
  // send it to the serial monitor
  if (hour<10)
  {
    DEBUG_PRINT("0");
  }
  DEBUG_PRINTDEC(hour);
  // convert the byte variable to a decimal number when displayed
  DEBUG_PRINT(":");
  if (minute<10)
  {
    DEBUG_PRINT("0");
  }
  DEBUG_PRINTDEC(minute);
  DEBUG_PRINT(":");
  if (second<10)
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
  DEBUG_PRINT(" Day of week: ");
  switch(dayOfWeek){
  case 1:
    DEBUG_PRINTLN("Monday");
    break;
  case 2:
    DEBUG_PRINTLN("Tuesday");
    break;
  case 3:
    DEBUG_PRINTLN("Wednesday");
    break;
  case 4:
    DEBUG_PRINTLN("Thursday");
    break;
  case 5:
    DEBUG_PRINTLN("Friday");
    break;
  case 6:
    DEBUG_PRINTLN("Saturday");
    break;
  case 7:
    DEBUG_PRINTLN("Sunday");
    break;
  }
}

void displayDebugTime(String inString, int statePinCase)
{
  DEBUG_PRINT(inString);
  DEBUG_PRINT(". Time: ");
  if (hour<10)
  {
    DEBUG_PRINT("0");
  }
  DEBUG_PRINTDEC(hour);
  // convert the byte variable to a decimal number when displayed
  DEBUG_PRINT(":");
  if (minute<10)
  {
    DEBUG_PRINT("0");
  }
  DEBUG_PRINTDEC(minute);
  DEBUG_PRINT(". ");
  switch (statePinCase)
  {
    case acLightPin:
       {
          if (acLightStartHour<10)
          {
            DEBUG_PRINT("0");
          }
          DEBUG_PRINTDEC(acLightStartHour);
          DEBUG_PRINT(":");
          if (acLightStartMins<10)
          {
            DEBUG_PRINT("0");
          }
          DEBUG_PRINTDEC(acLightStartMins);
          DEBUG_PRINT("-");
          if (acLightEndHour<10)
          {
            DEBUG_PRINT("0");
          }
          DEBUG_PRINTDEC(acLightEndHour);
          DEBUG_PRINT(":");
          if (acLightEndMins<10)
          {
            DEBUG_PRINT("0");
          }
          DEBUG_PRINTDEC(acLightEndMins);
          DEBUG_PRINT(". ");
        }
        break;
    case dcLightPin:
       {
          if (dcLightStartHour<10)
          {
            DEBUG_PRINT("0");
          }
          DEBUG_PRINTDEC(dcLightStartHour);
          DEBUG_PRINT(":");
          if (dcLightStartMins<10)
          {
            DEBUG_PRINT("0");
          }
          DEBUG_PRINTDEC(dcLightStartMins);
          DEBUG_PRINT("-");
          if (dcLightEndHour<10)
          {
            DEBUG_PRINT("0");
          }
          DEBUG_PRINTDEC(dcLightEndHour);
          DEBUG_PRINT(":");
          if (dcLightEndMins<10)
          {
            DEBUG_PRINT("0");
          }
          DEBUG_PRINTDEC(dcLightEndMins);
          DEBUG_PRINT(". ");
        }
        break;
     default:
      DEBUG_PRINT(" No Pin. ");
     break;
  }
}

void acLightOnOff()
{
  if (hour >= acLightStartHour && hour <= acLightEndHour) {
    if (hour == acLightStartHour && minute >= acLightStartMins) {
      acLightState = LOW;
      #ifdef DEBUG
        displayDebugTime("After Start Time", acLightPin);
      #endif
    } else if (hour == acLightEndHour && minute <= acLightEndMins) {
      acLightState = LOW;
      #ifdef DEBUG
        displayDebugTime("Before End Time", acLightPin);
      #endif
    } else if (hour > acLightStartHour && hour < acLightEndHour ) {
      #ifdef DEBUG
        displayDebugTime("Between Start & End Time", acLightPin);
      #endif
      acLightState = LOW;
    } else {
      #ifdef DEBUG
        displayDebugTime("Outside Time Minutes", acLightPin);
      #endif
      acLightState = HIGH;
    }
  } else {
      #ifdef DEBUG
        displayDebugTime("Outside Time Hours", acLightPin);
      #endif
    acLightState = HIGH;
  }
  if (holdAcLightState != acLightState) {
    digitalWrite(acLightPin, acLightState);
    holdAcLightState = acLightState;
  }
  DEBUG_PRINT(" AC Light. State = ");
  DEBUG_PRINTLN(acLightState);
}

void dcLightOnOff()
{
  if (hour >= dcLightStartHour && hour <= dcLightEndHour) {
    if (hour == dcLightStartHour && minute >= dcLightStartMins) {
      dcLightState = LOW;
      #ifdef DEBUG
        displayDebugTime("After Start Time", dcLightPin);
      #endif
    } else if (hour == dcLightEndHour && minute <= dcLightEndMins) {
      dcLightState = LOW;
      #ifdef DEBUG
        displayDebugTime("Before End Time", dcLightPin);
      #endif
    } else if (hour > dcLightStartHour && hour < dcLightEndHour ) {
      #ifdef DEBUG
        displayDebugTime("Between Start & End Time", dcLightPin);
      #endif
      dcLightState = LOW;
    } else {
      #ifdef DEBUG
        displayDebugTime("Outside Time Minutes", dcLightPin);
      #endif
      dcLightState = HIGH;
    }
  } else {
      #ifdef DEBUG
        displayDebugTime("Outside Time Hours", dcLightPin);
      #endif
    dcLightState = HIGH;
  }
  if (holdDcLightState != dcLightState) {
    digitalWrite(dcLightPin, dcLightState);
    holdDcLightState = dcLightState;
  }
  DEBUG_PRINT(" DC Light. State = ");
  DEBUG_PRINTLN(dcLightState);
}

boolean getTemperature() {
  //int LOWByte, HIGHByte, TReading, SignBit, Tc_100, Whole, Fract;
  int HIGHByte, LOWByte, TReading, SignBit;
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];

  ds.reset_search();
  if ( !ds.search(addr)) {
    ds.reset_search();
    return false;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
    return false;
  }

  if (!(( addr[0] == 0x10) || ( addr[0] == 0x28))) {
    return false;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  HIGHByte = data[0];
  LOWByte = data[1];
  TReading = (LOWByte << 8) + HIGHByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  if (SignBit){
    temperature = (TReading * 0.0625) * -1;
  } else {
    temperature = TReading * 0.0625;
  }
  return true;
}

void dcFanOnOffTempBased(){
  if (temperature > setTemperature) {
    dcFanState = LOW;
  } else {
    dcFanState = HIGH;
  }
  DEBUG_PRINT("DC Fan Temp Based: Temp = ");
  DEBUG_PRINT(temperature);
  DEBUG_PRINT(". State = ");
  DEBUG_PRINTLN(dcFanState);
//  digitalWrite(dcFanPin, dcFanState);
  if (holdDcFanState != dcFanState) {
    digitalWrite(dcFanPin, dcFanState);
    holdDcFanState = dcFanState;
//    DEBUG_PRINT("holdState = ");
//    DEBUG_PRINT(holdDcFanState);
//    DEBUG_PRINT(". Actual State = ");
//    DEBUG_PRINTLN(dcFanState);
  }
//    DEBUG_PRINT("holdState = ");
//    DEBUG_PRINT(holdDcFanState);
//    DEBUG_PRINT(". Actual State = ");
//    DEBUG_PRINTLN(dcFanState);
}

void dcFanOnOffLightBased(){
  if ((holdDcLightState == LOW) || (holdAcLightState == LOW)) {
    dcFanState = LOW;
  } else {
    dcFanState = HIGH;
  }
  DEBUG_PRINT("DC Fan Light Based, State = ");
  DEBUG_PRINTLN(dcFanState);
  if (holdDcFanState != dcFanState) {
    digitalWrite(dcFanPin, dcFanState);
    holdDcFanState = dcFanState;
  }
}

void dcFanArduinoOnOff(){
  if ((holdDcLightState == LOW) || (holdAcLightState == LOW)) {
    if ((minute >= 6 && minute <= 10) || (minute >= 21 && minute <= 25) || (minute >= 36 && minute <= 40) || (minute >= 51 && minute <= 55)){
      dcFanArduinoState = LOW;
    } else {
      dcFanArduinoState = HIGH;
    }
  } else {
    if (minute >= 5 && minute < 10) {
      dcFanArduinoState = LOW;
    } else {
      dcFanArduinoState = HIGH;
    }
  }
  DEBUG_PRINT("DC Fan Arduino. State =  ");
  DEBUG_PRINTLN(dcFanArduinoState);
  if (holdDcFanArduinoState != dcFanArduinoState) {
    digitalWrite(dcFanArduinoPin, dcFanArduinoState);
    holdDcFanArduinoState = dcFanArduinoState;
  }
}

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  DEBUG_SERIAL_BEGIN(9600);

  pinMode(acLightPin, OUTPUT);
  acLightState = HIGH;
  digitalWrite(acLightPin, acLightState);
  holdAcLightState = acLightState;
  
  pinMode(dcLightPin, OUTPUT);
  dcLightState = HIGH;
  digitalWrite(dcLightPin, dcLightState);
  holdDcLightState = dcLightState;

  pinMode(dcFanPin, OUTPUT);
  dcFanState = HIGH;
  digitalWrite(dcFanPin, dcFanState);
  holdDcFanState = dcFanState;

  pinMode(dcFanArduinoPin, OUTPUT);
  dcFanArduinoState = HIGH;
  digitalWrite(dcFanArduinoPin, dcFanArduinoState);
  holdDcFanArduinoState = dcFanArduinoState;
}

void loop() {
  // put your main code here, to run repeatedly:
  bool temperatureFound;

  // retrieve data from DS3231
  readDS3231time();
  #ifdef DEBUG
    displayTime();
  #endif

  acLightOnOff();

  dcLightOnOff();

  temperatureFound = getTemperature();
  if (temperatureFound) {
    dcFanOnOffTempBased();
  } else {
    dcFanOnOffLightBased();
  }

  dcFanArduinoOnOff();
  
  delay(sleepTimeInSec * 1000);
}
