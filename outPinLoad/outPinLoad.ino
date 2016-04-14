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
        int     dcLightState          = HIGH;
        int     holdDcLightState      = HIGH;

const   int     acLightStartHour      = 8;
const   int     acLightStartMins      = 1;
const   int     acLightEndHour        = 15;
const   int     acLightEndMins        = 59;
        int     acLightState          = HIGH;
        int     holdAcLightState      = HIGH;

        int     dcFanState            = HIGH;
        int     holdDcFanState        = HIGH;
const   float   setTemperature        = 28.0;
        float   temperature;

        int     acMotorState          = HIGH;
        int     holdAcMotorState      = HIGH;
        int     holdFloatSwitchState  = LOW;  // assume floatSwitch open because of pull-up resistor
        int     floatSwitchState      = LOW;
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
      dcLightState = LOW;
    } else if (*hour == dcLightEndHour && *minute <= dcLightEndMins) {
      dcLightState = LOW;
    } else if (*hour > dcLightStartHour && *hour < dcLightEndHour ) {
      dcLightState = LOW;
    } else {
      dcLightState = HIGH;
    }
  } else {
    dcLightState = HIGH;
  }
  DEBUG_PRINT("Function DC Light ");
  DEBUG_PRINT(holdDcLightState);
  DEBUG_PRINT(" ");
  DEBUG_PRINTLN(dcLightState);
  if (holdDcLightState != dcLightState) {
    digitalWrite(dcLightPin, dcLightState);
    holdDcLightState = dcLightState;
    DEBUG_PRINT("Write DC Light ");
    DEBUG_PRINT(holdDcLightState);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN(dcLightState);
  }
//    if (dcLightState == LOW) {
//      dcLightState = HIGH;
//      digitalWrite(dcLightPin, LOW);
//    }
}

void acLightOnOff(byte *hour,
                  byte *minute){
  
  if (*hour >= acLightStartHour && *hour <= acLightEndHour) {
    if (*hour == acLightStartHour && *minute >= acLightStartMins) {
      acLightState = LOW;
    } else if (*hour == acLightEndHour && *minute <= acLightEndMins) {
      acLightState = LOW;
    } else if (*hour > acLightStartHour && *hour < acLightEndHour ) {
      acLightState = LOW;
    } else {
      acLightState = HIGH;
    }
  } else {
    acLightState = HIGH;
  }
  DEBUG_PRINT("AC Light ");
  DEBUG_PRINTLN(dcLightState);
  if (holdAcLightState != acLightState) {
    digitalWrite(acLightPin, acLightState);
    holdAcLightState = acLightState;
  }
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
  DEBUG_PRINT("DC Fan Temp Based ");
  DEBUG_PRINTLN(dcFanState);
  if (holdDcFanState != dcFanState) {
    digitalWrite(dcFanPin, dcFanState);
    holdDcFanState = dcFanState;
  }
}

void dcFanOnOffLightBased(){
  if ((holdDcLightState == LOW) || (holdAcLightState == LOW)) {
    dcFanState = LOW;
  } else {
    dcFanState = HIGH;
  }
  DEBUG_PRINT("DC Fan Light Based ");
  DEBUG_PRINTLN(dcFanState);
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
      if (floatSwitchState == HIGH)
      {
        acMotorState = LOW;
        DEBUG_PRINTLN("AC Motor LOW");
      }  // end if floatSwitchState is HIGH
      else
      {
        acMotorState = HIGH;
        DEBUG_PRINTLN("AC Motor HIGH");
      }  // end if floatSwitchState is LOW
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
  DEBUG_SERIAL_BEGIN(9600);
  
  pinMode(dcLightPin, OUTPUT);
  dcLightState = HIGH;
  digitalWrite(dcLightPin, dcLightState);
  holdDcLightState = dcLightState;
  DEBUG_PRINT("Setup DC LIGHT ");
  DEBUG_PRINTLN(dcLightState);
  
  pinMode(acLightPin, OUTPUT);
  digitalWrite(acLightPin, acLightState);
  holdAcLightState = acLightState;
  
  pinMode(dcFanPin, OUTPUT);
  digitalWrite(dcFanPin, dcFanState);
  holdDcFanState = dcFanState;
  
  pinMode(floatSwitchPin, INPUT_PULLUP);
  floatSwitchState = digitalRead (floatSwitchPin);  //get Value here because of PULL UP Register.
  holdFloatSwitchState =  floatSwitchState;
  
  pinMode(acMotorPin, OUTPUT);
  digitalWrite(acMotorPin, acMotorState);
  holdAcMotorState = acMotorState;
}

void loop() {
  // put your main code here, to run repeatedly:
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  bool temperatureFound;
  
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  
  DEBUG_PRINT("LOOP DC LIGHT ");
  DEBUG_PRINTDEC(hour);
  DEBUG_PRINTDEC(minute);
  DEBUG_PRINTLN("");
  dcLightOnOff(&hour, &minute);
  
  acLightOnOff(&hour, &minute);

  temperatureFound = getTemperature();
  if (temperatureFound) {
    dcFanOnOffTempBased();
    DEBUG_PRINTLN(temperature);
  } else {
    dcFanOnOffLightBased();
  }
  checkFloatSwitch();
  if (acMotorState == HIGH){
    delay(sleepTimeInSec * 1000);
  }
}
