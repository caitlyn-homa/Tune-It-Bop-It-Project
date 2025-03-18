//Tuner_Task
//Sawyer Mervis 3-12-2025
//

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(8, OUTPUT);
}

void loop() {
  // read the value from the sensor:
  
  // turn the ledPin on
  digitalWrite(8, HIGH);
  // stop the program for <sensorValue> milliseconds:
  delay(1000);
  // turn the ledPin off:
  digitalWrite(8, LOW);
  // stop the program for <sensorValue> milliseconds:
  bool result = tuner(1000);

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

bool tuner(int time_lim)
{
  int thresh = 50;
  int starttime = millis();
  int endtime = starttime;
  int start_voltage = analogRead(A0);
  while ((endtime - starttime) <= time_lim) // do this loop for up to 1000mS
  {
    int current_voltage = analogRead(A0);
    if(abs(current_voltage - start_voltage) > thresh)
      {
        return true;
      }

  //loopcount = loopcount+1;
  endtime = millis();
  }
  return false;
}
