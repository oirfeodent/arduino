#include <Wire.h>
#include <OneWire.h>

#define DS3231_I2C_ADDRESS 0x68

//All PINs to know available pins
OneWire ds(10);  // on pin 10
const   int     acMotorPin            = 12;
const   int     dcLightPin            = 7;
const   int     acLightPin            = 2;
const   int     dcFanPin              = 4;
const   int     floatSwitchPin        = 8;

const   int     dcLightStartHour      = 8;
const   int     dcLightStartMins      = 15;
const   int     dcLightEndHour        = 15;
const   int     dcLightEndMins        = 45;
        int     dcLightState          = LOW;
        int     holdDcLightState      = LOW;

const   int     acLightStartHour      = 8;
const   int     acLightStartMins      = 1;
const   int     acLightEndHour        = 15;
const   int     acLightEndMins        = 59;
        int     acLightState          = LOW;
        int     holdAcLightState      = LOW;

        int     dcFanState            = LOW;
        int     holdDcFanState        = LOW;
const   float   setTemperature        = 28.0;
        float   temperature;

        int     acMotorState          = LOW;
        int     holdAcMotorState      = LOW;
        int     holdFloatSwitchState  = HIGH;  // assume floatSwitch open because of pull-up resistor
        int     floatSwitchState      = HIGH;
const unsigned long debounceTime = 10;  // milliseconds
      unsigned long floatSwitchPressTime;  // when the floatSwitch last changed state
      
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

void dcLightOnOff(byte *hour,
                  byte *minute){
  
  if (*hour >= dcLightStartHour && *hour <= dcLightEndHour) {
    if (*hour == dcLightStartHour && *minute >= dcLightStartMins) {
      dcLightState = HIGH;
    } else if (*hour == dcLightEndHour && *minute <= dcLightEndMins) {
      dcLightState = HIGH;
    } else if (*hour > dcLightStartHour && *hour < dcLightEndHour ) {
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

void acLightOnOff(byte *hour,
                  byte *minute){
  
  if (*hour >= acLightStartHour && *hour <= acLightEndHour) {
    if (*hour == acLightStartHour && *minute >= acLightStartMins) {
      acLightState = HIGH;
    } else if (*hour == acLightEndHour && *minute <= acLightEndMins) {
      acLightState = HIGH;
    } else if (*hour > acLightStartHour && *hour < acLightEndHour ) {
      acLightState = HIGH;
    } else {
      acLightState = LOW;
    }
  } else {
    acLightState = LOW;
  }
  if (holdAcLightState != acLightState) {
    digitalWrite(acLightPin, acLightState);
    holdAcLightState = acLightState;
  }
}

boolean getTemperature() {
  //int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;
  int LowByte, HighByte, TReading, SignBit;
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
  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
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
    dcFanState = HIGH;
  } else {
    dcFanState = LOW;
  }
  if (holdDcFanState != dcFanState) {
    digitalWrite(dcFanPin, dcFanState);
    holdDcFanState = dcFanState;
  }
}

void dcFanOnOffLightBased(){
  if ((holdDcLightState == HIGH) || (holdAcLightState == HIGH)) {
    dcFanState = HIGH;
  } else {
    dcFanState = LOW;
  }
  if (holdDcFanState != dcFanState) {
    digitalWrite(dcFanPin, dcFanState);
    holdDcFanState = dcFanState;
  }
}

void checkFloatSwitch() {
  // see if floatSwitch is open or closed
  floatSwitchState = digitalRead (floatSwitchPin);

  // has it changed since last time?
  if (floatSwitchState != holdFloatSwitchState)
  {
    // debounce
    if (millis () - floatSwitchPressTime >= debounceTime)
    {
      floatSwitchPressTime = millis ();  // when we closed the floatSwitch
      holdFloatSwitchState =  floatSwitchState;  // remember for next time
      if (floatSwitchState == LOW)
      {
        acMotorState = HIGH;
      }  // end if floatSwitchState is LOW
      else
      {
        acMotorState = LOW;
      }  // end if floatSwitchState is HIGH
      if (holdAcMotorState != acMotorState) {
        digitalWrite(acMotorPin, acMotorState);
        holdAcMotorState = acMotorState;
      }
    }  // end if debounce time up
  }  // end of state change
}

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  pinMode(dcLightPin, OUTPUT);
  pinMode(acLightPin, OUTPUT);
  pinMode(dcFanPin, OUTPUT);
  pinMode(floatSwitchPin, INPUT_PULLUP);
  floatSwitchState = digitalRead (floatSwitchPin);  //get Value here because of PULL UP Register.
  holdFloatSwitchState =  floatSwitchState;
  pinMode(acMotorPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  bool temperatureFound;
  
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  dcLightOnOff(&hour, &minute);
  acLightOnOff(&hour, &minute);

  temperatureFound = getTemperature();
  if (temperatureFound) {
    dcFanOnOffTempBased();
  } else {
    dcFanOnOffLightBased();
  }
}
