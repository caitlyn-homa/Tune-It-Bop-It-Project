#include <DFMiniMp3.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include "SparkFunLIS3DH.h" 

//Keep track of player's score throughout the game
byte score = 0;
//Initial time player has to complete a task [seconds]
int time_to_complete = 4;
//Keeps track whether action was successfully completed or not 
bool isComplete = false;
//Keeps track of what round player is on - will be used to exit loop if player loses 
byte iteration = 0;
//Flag for if game is restarting 
bool restartPressed = false;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
LIS3DH SensorOne( I2C_MODE, 0x19 );

void setup() {
  //Seed random number generation with current time -ensures variation in the sequence of random numbers 
  randomSeed(analogRead(A1));

  //Pin Set-Up 
  // Reset/Start button
  pinMode(8, OUTPUT);
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(3), restart_game, LOW);
  // Pull Task
  pinMode(2, INPUT_PULLUP);
  // Tune Task
  pinMode(A0, INPUT);
  // Shake Task
  pinMode(A5, INPUT);
  // Stepper Motor 
  pinMode(5, OUTPUT); //st_a
  pinMode(6, OUTPUT); //st_b
  pinMode(7, OUTPUT); //st_c
  pinMode(8, OUTPUT); //st_d
  // OLED Display
  pinMode(A4, OUTPUT);  
  // Speaker 
  pinMode(0, INPUT);
  pinMode(1, OUTPUT);

  // Volume 
  pinMode(A3, INPUT);

  //OLED set-up 
  oled_set_up();

}

void loop() {

  oled_display_str("Press Start", 2);

  //If start of new game 
  if(restartPressed)
  {
    //Reset start button flag 
    restartPressed = false;

    //Put score motor back to far right
    motor_reset();          
  
    //Indicate to player game is about to begin and have a countdown 
    oled_display_str("Get Ready!", 2);
    oled_countdown(3);

    //Start music 

    //Game loop - Player wins after 99 rounds 
    for(iteration = 0; iteration<99; iteration++)
    {
      //Reset action completion variable back to false
      isComplete = false;

      //Generates random number 1 through 3 to select action - MAP -> #1 = Pull-it #2 = Tune-it #3 = Shake-it
      byte action_number = random(1,4);

      //Pull-It! 
      if(action_number == 1)
      {
        //Cut out sound/music
        //Play Pull it! sound
        oled_display_str("Pull It!", 2);
        delay(200);
        isComplete = has_pulled(time_to_complete);

        //If action is completed within the time 
        if (isComplete)
        {
          //Continue music 
          score++; 
          rotate_motor(1,2);
          delay(500);
        } 
        else {
          //Go to steps to take if they lost 
          player_lost(score);
        }
      }
      //Tune-It!
      if(action_number == 2)
      {
        //Change music/cut out 
        //Play Tune it! sound
        oled_display_str("Tune It!", 2);
        delay(500);
        isComplete = has_tuned(time_to_complete);

        //If action is completed within the time 
        if (isComplete)
        {
          //Continue music 
          score++; 
          rotate_motor(1,2);
          delay(500);
        } 
        else {
          //Cue loss protocol
          player_lost(score);
        }
      }
      //Shake-It!
      if(action_number == 3)
      {
        //Play static sound 
        oled_display_str("Shake It!", 2);
        delay(500);
        isComplete = has_shaken(time_to_complete);

        //If action is completed within the time 
        if (isComplete)
        {
          //Continue music 
          score++; 
          rotate_motor(1,2);
          delay(500);
        } 
        else {
          //Cue loss protocol
          player_lost(score);
        }
      }
      //End of actions 

      //Decrease time to complete action every 5th iteration or so - for 99 iterations - will decrease 19 times
      if(iteration % 5 == 0)
      {
        //Will decrease by 2.85 seconds if player makes it to the end 
        time_to_complete = time_to_complete - 0.15;
      }

    } //end of for loop

    //Player has won - indicate the win somehow
    if(score == 99)
    {
      player_won();
    }
  }

  //Reset iteration to 0 to start new game
  iteration = 0;
}

//Sets restart flag to true to know when to begin a new game 
void restart_game()
{
  score = 0;              //Make sure score is at 0 
  time_to_complete = 4;   //Reset initial time to complete action 

  //If currently in game 
  if(iteration > 0)
  {
    //Exit game and set restart back to false - player will have to press the button again to start a new game
    iteration = 101;
    restartPressed = false;
    oled_display_str("Restarting", 2);
  }
  //Else, new game is about to begin
  else
  {
    restartPressed = true;
  }
  
}

//IN-GAME FUNCTIONS 

//Steps to follow if player has lost 
void player_lost(byte score)
{
  oled_display_str("You Lost", 3);
  delay(2000); 
  //Play broken radio sounds 

  oled_display_str("Final Score:", 3);
  delay(2000);
  oled_display_score(score);
  delay(2000); 

  //Set iteration to 101 to exit game loop
  iteration = 101;
}

//Steps to follow if player has won
void player_won()
{
  oled_display_str("Congrats!", 3);
  delay(2000);
  oled_display_str("You Win!", 3);
  delay(2000);

  //Play sweet caroline 

}

//Returns true or false depending if player has succesfully shaken the radio
//Parameter - time_lime = time player has to complete the task 
bool has_shaken(int time_lim)
{
  //Convert time limit to milliseconds
  time_lim = time_lim * 1000;

  //Magnitude of acceleration that player must meet to succesfully complete shake it task
  float threshold = 1.25;

  //Begin countdown 
  int starttime = millis();
  int endtime = starttime;
  while ((endtime - starttime) <= time_lim) //Runs until time limit or response
  {
    //Read sensor data for accleration in x, y, and z directions 
    float a_x = SensorOne.readFloatAccelX();
    float a_y = SensorOne.readFloatAccelY();
    float a_z = SensorOne.readFloatAccelZ();

    //Calculate the accleration magnitude detected
    float mag = sqrt(a_x*a_x + a_y*a_y + a_z*a_z);

    //If accleration is greater than threshold - player has passed
    if(mag > threshold)
    {
      return true;
    }
    
    //Update time passed
    endtime = millis();
    }

  //Player failed 
  return false;
}

//Returns true or false depending if player has succesfully pulled the antenna 
//Parameter - time_lime = time player has to complete the task 
bool has_pulled(int time_lim)
{
  //Convert time limit to milliseconds
  time_lim = time_lim * 1000;

  //Set up countdown variables and begin the timer
  int starttime = millis();
  int endtime = starttime;

  //Get button's inital state 
  int initState = digitalRead(2);
  while ((endtime - starttime) <= time_lim) //Runs until time limit or response
  {
    //Check button's state - Has player succesfully pulled the antenna
    int buttonState = digitalRead(2);

    //If transistion in button's state is detected -> player has succeeded
    if((buttonState - initState) != 0) 
      {
        return true;
      }
  
  //Update time passed
  endtime = millis();
  }

  //Player failed 
  return false;
}

//Returns true or false depending on if player has succesfully tuned the knob 
//Parameter - time_lime = time player has to complete the task 
bool has_tuned(int time_lim)
{
  //Convert time limit to milliseconds
  time_lim = time_lim * 1000;

  //Set a threshold that the knob must be turned to for a successful attempt 
  int thresh = 50;

  //Obtain current time and begin the countdown  
  int starttime = millis();
  int endtime = starttime;

  //Read the starting voltage at the tuning knob 
  int start_voltage = analogRead(A0);

  while ((endtime - starttime) <= time_lim) // do this loop for up to 1000mS
  {
    //Read current voltage at the tuning knob 
    int current_voltage = analogRead(A0);
    //If the difference is greater than the threshold - task is completed 
    if(abs(current_voltage - start_voltage) > thresh)
      {
        return true;
      }

  //Update time passed
  endtime = millis();
  }

  //Player failed
  return false;
}

//OLED FUNCTIONS 

//Sets up the OLED and displays Tune It! message 
void oled_set_up(){

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  delay(200);
  display.clearDisplay();

  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Tune It!");
  display.display(); 

  Serial.begin(9600);
  delay(100); 
  Serial.println("Processor came out of reset.\n");
  
  //Call .begin() to configure the IMUs
  if( SensorOne.begin() != 0 )
  {
	  Serial.println("Problem starting the sensor at 0x19.");
  }
  else
  {
	  Serial.println("Sensor at 0x19 started.");
  }
}

//Displays score on oled - always at size 4 
void oled_display_score(byte score){

  delay(10);
  display.clearDisplay();
  display.setTextSize(4);

  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println(score);
  display.display(); 

}

//Display text to oled at specified text size 
void oled_display_str(char* disp, byte text_size){

  delay(10);
  display.clearDisplay();
  display.setTextSize(text_size);

  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println(disp);
  display.display(); 

}

//Visual countdown on oled for whenn game is about to begin
void oled_countdown(int time) 
{
  time = time * 1000;  // Convert time to milliseconds
  int starttime = millis();
  int endtime = starttime;
  int lastUpdateTime = starttime;  // To track when to update the display

  while ((endtime - starttime) < time) {  // Run until the time limit is reached
    endtime = millis();  // Update the current time

    if (endtime - lastUpdateTime >= 1000) {  // If 1 second has passed
      lastUpdateTime = endtime;  // Update the last update time
      int secondsPassed = (endtime - starttime) / 1000;  // Calculate the seconds passed
      oled_display_score(secondsPassed);  // Update the display
    }
  }
}

// MOTOR FUNCTIONS 

//Rotates motor needs specific time delay step and angle 
void rotate_motor(int t_step, int steps)
{
  //Pins where motor is set up at 
  int st_a = 5;
  int st_b = 6;
  int st_c = 7;
  int st_d = 8;
  
  //Drive the stepper motor using appropriate pins for each step 
  for(int i = 0; i < steps; ++i)
  {
    digitalWrite(st_a, LOW); //A
    digitalWrite(st_b, HIGH);
    digitalWrite(st_c, HIGH);
    digitalWrite(st_d, HIGH);
    delay(t_step);
    digitalWrite(st_a, LOW); //AB
    digitalWrite(st_b, LOW);
    digitalWrite(st_c, HIGH);
    digitalWrite(st_d, HIGH);
    delay(t_step);
    digitalWrite(st_a, HIGH); //B
    digitalWrite(st_b, LOW);
    digitalWrite(st_c, HIGH);
    digitalWrite(st_d, HIGH);
    delay(t_step);
    digitalWrite(st_a, HIGH); //BC
    digitalWrite(st_b, LOW);
    digitalWrite(st_c, LOW);
    digitalWrite(st_d, HIGH);
    delay(t_step);
    digitalWrite(st_a, HIGH); //C
    digitalWrite(st_b, HIGH);
    digitalWrite(st_c, LOW);
    digitalWrite(st_d, HIGH);
    delay(t_step);
    digitalWrite(st_a, HIGH); //CD
    digitalWrite(st_b, HIGH);
    digitalWrite(st_c, LOW);
    digitalWrite(st_d, LOW);
    delay(t_step);
    digitalWrite(st_a, HIGH); //D
    digitalWrite(st_b, HIGH);
    digitalWrite(st_c, HIGH);
    digitalWrite(st_d, LOW);
    delay(t_step);
    digitalWrite(st_a, LOW); //AD
    digitalWrite(st_b, HIGH);
    digitalWrite(st_c, HIGH);
    digitalWrite(st_d, LOW);
    delay(t_step);
  }
}

//Resets motor back to starting position (far right)
void motor_reset()
 {
   int st_a = 5;
   int st_b = 6;
   int st_c = 7;
   int st_d = 8;
   int t_step = 2;
   while(digitalRead(4)==0)
   {
     digitalWrite(st_a, LOW); //AD
     digitalWrite(st_b, HIGH);
     digitalWrite(st_c, HIGH);
     digitalWrite(st_d, LOW);
     delay(t_step);
     digitalWrite(st_a, HIGH); //D
     digitalWrite(st_b, HIGH);
     digitalWrite(st_c, HIGH);
     digitalWrite(st_d, LOW);
     delay(t_step);
     digitalWrite(st_a, HIGH); //CD
     digitalWrite(st_b, HIGH);
     digitalWrite(st_c, LOW);
     digitalWrite(st_d, LOW);
     delay(t_step);
     digitalWrite(st_a, HIGH); //C
     digitalWrite(st_b, HIGH);
     digitalWrite(st_c, LOW);
     digitalWrite(st_d, HIGH);
     delay(t_step);
     digitalWrite(st_a, HIGH); //BC
     digitalWrite(st_b, LOW);
     digitalWrite(st_c, LOW);
     digitalWrite(st_d, HIGH);
     delay(t_step);
     digitalWrite(st_a, HIGH); //B
     digitalWrite(st_b, LOW);
     digitalWrite(st_c, HIGH);
     digitalWrite(st_d, HIGH);
     delay(t_step);
     digitalWrite(st_a, LOW); //AB
     digitalWrite(st_b, LOW);
     digitalWrite(st_c, HIGH);
     digitalWrite(st_d, HIGH);
     delay(t_step);
     digitalWrite(st_a, LOW); //A
     digitalWrite(st_b, HIGH);
     digitalWrite(st_c, HIGH);
     digitalWrite(st_d, HIGH);
     delay(t_step);
   }
 }
 
