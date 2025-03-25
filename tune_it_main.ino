#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>

//#include "SparkFunLIS3DH.h"


//RESET Button variables
// constants won't change. They're used here to set pin numbers:
const int buttonPin = 0;  // the number of the pushbutton pin
const int ledPin = 8;    // the number of the LED pin
const byte interruptPin = 3;

//Other variables
int buttonState = 0;  // variable for reading the pushbutton status
int score = 0;  // keep track of score 
int iteration = 0; // keep track of game loop iteration 
bool actionCompleted = false // holds if player has completed task of not 

//Countdown variables 
unsigned long previous_time = 0 //holds previous time and how many times countdown has been updated
unsigned long current_time = 0 //using millis() to get current time to begin a timer
const long conversion = 1000 //1000 miliseconds in a second
int interval = 3; // holds time to complete action (in seconds)
int countdown = interval; // will be countdown variable when action is called 

void startGame();
void lose();
void win();
void reset_pressed();
bool pull(int time_lim);

//OLED DISPLAY set-up 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//LIS3DH SensorOne( I2C_MODE, 0x19 );

void setup() {
  //Initialize communication with computer from microprocessor 
  Serial.begin(9600);
  //Seed random number generation 
  randomSeed(analogRead(0));

  //PULL TASK set-up 
   // initialize the LED pin as an output:
  pinMode(8, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(2, INPUT_PULLUP);

  //TUNER TASK set-up - declare the ledPin as an OUTPUT 
  pinMode(8, OUTPUT);

  //SHAKE TASK set-up 
  pinMode(8, OUTPUT);

  //STEPPER MOTOR set-up
  int st_a = 5;
  int st_b = 6;
  int st_c = 7;
  int st_d = 8;

  pinMode(st_a, OUTPUT);
  pinMode(st_b, OUTPUT);
  pinMode(st_c, OUTPUT);
  pinMode(st_d, OUTPUT);
  pinMode(0, INPUT_PULLUP);

  //RESET BUTTON set-up
  // initialize the LED pin as an output:
  pinMode(8, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(0, INPUT_PULLUP);

  //digital pin 3 to trigger an interrupt to reset the game - will run reset_pressed protocol when it detects a change at digital pin 3
  attachInterrupt(digitalPinToInterrupt(interruptPin), reset_pressed, CHANGE);


}

//MAIN LOOP
void loop() 
{
  //If start button has been pushed (pin #)
    startGame()

    //Loop over 99 times 
    for(iteration = 0; iteration<99; iteration++) {
      //Should generate a random number between 1 and 3 MAPPING -> #1 = Pull the Antenna #2 = Tune the Radio #3 = Shake the Radio
      action = random(1,4);

      //Run through Pull the Antenna action sequences
      if(action == 1)
      {
        //Cut out radio song 
        //Play Pull it! sound 
        //Begin countdown by obtaining the current time
        current_time = millis()

        //If a second hasn't passed 
        if(current_time - previous_time >= conversion)
        {
          previous_time = current_time 

          //And if there is still time left and action is not yet completed --> 
          if(countdown >= 0 && !actionCompleted)
          {
            countdown--;
          }
          //Else countdown hit zero and player did not complete task
            else
            {
              //Player has lost
              lose()
            }
        }
    }

    //Decrease interval every 5th iteration or so - for 99 iterations - will decrease 19 times 
    if(iteration % 5 == 0){
      //Decrease countdown interval -- decrease a total of 2.85 seconds if player makes it to the end 
      interval = interval - 0.15
      //Reset countdown back to interval 
      countdown = interval
    }
    else {
      //Reset countdown back to interval
      countdown = interval
    }
  }


}

//Function to reset back to start of game conditions
void startGame() {

  int score = 0;  // keep track of score 
  int iteration = 0; // keep track of game loop iteration 
  int interval = 0; // holds time to complete action
  int countdown = 0; // will countdown when action is called 

  //Reset motor too 

  //Play indication game will begin 

  return;
}

//Function to carry out steps when player has lost 
void lose() {
  
  iteration = 100; //Set iteration to 100 to not confuse with a win but exits game loop 
  //indicate loss 
  //display final score 

  return;
}

//Function to carry out the steps when player has won 
void win(){
  //play winning sound - sweet caroline? 

  return;
}

void reset_pressed(){

  //indicate reset has been pressed somehow
  iteration = 101;

  return;

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


