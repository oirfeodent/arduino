const byte floatSwitchPin = 8;
byte holdFloatSwitchState = HIGH;  // assume floatSwitch open because of pull-up resistor
byte floatSwitchState = HIGH;
const unsigned long debounceTime = 10;  // milliseconds
unsigned long floatSwitchPressTime;  // when the floatSwitch last changed state

int timesChanged;

void setup ()
{
  Serial.begin (9600);
  pinMode (floatSwitchPin, INPUT_PULLUP);
  floatSwitchState = digitalRead (floatSwitchPin);
  holdFloatSwitchState =  floatSwitchState;

  Serial.println(floatSwitchState);
  
  timesChanged = 0;
}  // end of setup

void loop ()
{
  checkFloatSwitch();
}  // end of loop

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
        timesChanged++;
        Serial.print(timesChanged);
        Serial.print(": ");
        Serial.print(floatSwitchState);
        Serial.print(": ");
        Serial.println ("floatSwitch closed.");
      }  // end if floatSwitchState is LOW
      else
      {
        timesChanged--;
        Serial.print(timesChanged);
        Serial.print(": ");
        Serial.print(floatSwitchState);
        Serial.print(": ");
        Serial.println ("floatSwitch opened.");
      }  // end if floatSwitchState is HIGH
    }  // end if debounce time up
  }  // end of state change
}

