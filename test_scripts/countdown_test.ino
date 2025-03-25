//Test to see if countdown is working - uses method that allows other tasks to occur while countdown is going

//Countdown variables 
unsigned long previous_time = 0; //holds previous time and how many times countdown has been updated
unsigned long current_time = 0; //using millis() to get current time to begin a timer
const long time_elasped = 100; //100 miliseconds --> 0.01 seconds 
byte interval = 3; // holds time to complete action (in seconds)
byte countdown = interval; // will be countdown variable when action is called 

void setup() {
  //Initialize communication with computer from microprocessor 
  Serial.begin(9600);

  //Set digital pin 8 as an output for LED to light up when count down is complete 
  pinMode(8, OUTPUT);

}

void loop() {

  for(byte i = 0; i<99; i++)
  {
    //Get current time in milliseconds 
    current_time = millis();

    //Check time elasped -> once 100 milliseconds passed 
    if(current_time - previous_time >= time_elasped)
    {
      //Resets starting interval for next steps 
      previous_time = current_time;

      //If countdown still has time remaining aka not zero 
      if(countdown >= 0)
      {
        Serial.println(countdown);
        //Decrease countdown by 0.01 seconds - useful when interval is not an integer 
        countdown = countdown = 0.01;
      }
        //Time is up - light up the LED
        else
        {
          digitalWrite(8, HIGH);
          delay(1000);
          digitalWrite(8,LOW);
          delay(1000);
        }
    }

    //Decrease interval every 5th iteration or so -- for 99 iterations - will decrease 19 times
    if(i%5 == 0)
    {
      //Decrease countdown interval -- decrease a total of 2.85 seconds if player makes it to the end 
      interval = interval - 0.15;
      //Reset countdown back to interval 
      countdown = interval;
    }
    else
    {
      //Reset countdown back to interval 
      countdown = interval;
    }
  }

}
