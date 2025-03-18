/*
  Button

  Turns on and off a light emitting diode(LED) connected to digital pin 13,
  when pressing a pushbutton attached to pin 2.

  The circuit:
  - LED attached from pin 13 to ground through 220 ohm resistor
  - pushbutton attached to pin 2 from +5V
  - 10K resistor attached to pin 2 from ground

  - Note: on most Arduinos there is already an LED on the board
    attached to pin 13.

  created 2005
  by DojoDave <http://www.0j0.org>
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Button
*/

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 2;  // the number of the pushbutton pin
const int ledPin = 8;    // the number of the LED pin

// variables will change:
int buttonState = 0;  // variable for reading the pushbutton status

void setup() {
  // initialize the LED pin as an output:
  pinMode(8, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(2, INPUT_PULLUP);
}

void loop() {
  // turn the ledPin on
  digitalWrite(8, HIGH);
  // stop the program for <sensorValue> milliseconds:
  delay(1000);
  // turn the ledPin off:
  digitalWrite(8, LOW);
  // stop the program for <sensorValue> milliseconds:
  bool result = pull(1000);

  if(result)
  {
    digitalWrite(8, HIGH);
    // stop the program for <sensorValue> milliseconds:
    delay(100);
    // turn the ledPin off:
    digitalWrite(8, LOW);
    delay(100);
    digitalWrite(8, HIGH);
    // stop the program for <sensorValue> milliseconds:
    delay(100);
    // turn the ledPin off:
    digitalWrite(8, LOW);
    delay(1000);
  }
  else
  {
    digitalWrite(8, HIGH);
    // stop the program for <sensorValue> milliseconds:
    delay(200);
    // turn the ledPin off:
    digitalWrite(8, LOW);
    delay(1000);
  }

}

bool pull(int time_lim)
{
  int starttime = millis();
  int endtime = starttime;
  int initState = digitalRead(2);
  while ((endtime - starttime) <= time_lim) //Runs until time limit or response
  {
    int buttonState = digitalRead(2);
    if((buttonState - initState) != 0) //Detects transision
      {
        return true;
      }

  endtime = millis();
  }
  return false;
}