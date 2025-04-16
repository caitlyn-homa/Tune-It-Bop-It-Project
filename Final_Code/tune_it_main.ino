#include <DFMiniMp3.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include "SparkFunLIS3DH.h" 
#include "Mp3Manager.hpp"

//OLED Variables & Object
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Accelerometer Object
LIS3DH SensorOne(I2C_MODE, 0x19);
//SD Card Object
Mp3Manager mp3;

//In-Game Variables 
byte score = 0;
byte iteration = 0;
bool isComplete = false;
volatile bool restartPressed = false;
int numRounds = 99;
float time_to_complete = 4.0;
//Used to calculate time interval decrease - following exponential decay
float time_constant = -numRounds/log(0.7/4); 

int volume = analogRead(A3)/20.5;

//Used to debug when having memory issues 
extern int __heap_start, *__brkval;
int freeMemory() {
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void setup() {
  pinMode(3, INPUT_PULLUP);   // Start button
  attachInterrupt(digitalPinToInterrupt(3), restart_game, FALLING);
  pinMode(4, INPUT_PULLUP);   // Motor Reset
  pinMode(2, INPUT_PULLUP);   // Pull sensor
  pinMode(A0, INPUT);         // Potentiometer

  // Stepper motor pins
  for (int i = 5; i <= 8; i++) pinMode(i, OUTPUT);

  //Oled Set-up
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  //Accelerometer Set-up
  SensorOne.begin();
}

//Main Game Loop
void loop() {
  waitMilliseconds(100);
  oled_display_str(F("Press"), 2, 0, true);
  oled_display_str(F("Start"), 2, 1, false);

  //If game is about to start - will jump to ISR first 
  if (restartPressed) {
    restartPressed = false;   //Reset flag if start has been pushed 
    mp3.begin();              //Begin song 
    waitMilliseconds(10);
    motor_reset();            //Put score dial back 
    oled_display_str(F("Get Ready!"), 2, 0, true);  //Warn player the game will start soon
    oled_countdown(3);

    //Start executing the 99 rounds of the game 
    for (iteration = 0; iteration < numRounds; iteration++) {
      mp3.loop();       //Keeps song playing 
      isComplete = false;
      byte action_number = random(1, 4);

      if (action_number == 1) {                        //PULL-IT == 1
        oled_display_str(F("Pull It!"), 2, 0, true);
        waitMilliseconds(300);
        isComplete = has_pulled(time_to_complete);
      } else if (action_number == 2) {                 //TUNE-IT == 2
        oled_display_str(F("Tune It!"), 2, 0, true);
        waitMilliseconds(300);
        isComplete = has_tuned(time_to_complete);
      } else {
        oled_display_str(F("Shake It!"), 2, 0, true);  //SHAKE-IT == 3
        waitMilliseconds(300);
        isComplete = has_shaken(time_to_complete);
      }

      //If player was successful 
      if (isComplete) {
        score++;                //Increase score
        mp3.setVolume(volume);  //Restore the audio 
        oled_display_str(F("Signal"), 2, 0, true);    
        oled_display_str(F("Connected!"), 2, 1, false);
        rotate_motor(1, 2);    //Move score dial 
        waitMilliseconds(600);
      } else { //The player was unsuccessful 
        player_lost(score);
        break; //Exit the game - do not start next round 
      }

      //Update game time according to the exponetial decay - using iteration as "time" passed
      time_to_complete = 4 * exp(-(iteration+1)/time_constant);
    }
    
    //Player wins if they score 99 points 
    if (score == numRounds) player_won();
    iteration = 0; //Reset game iteration 
  }
}

//ISR when start has been pushed 
void restart_game() {
  //Debounce 
  static unsigned long lastInterruptTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > 300) {
    lastInterruptTime = currentTime;  
    score = 0;              //Reset game variables 
    time_to_complete = 4;
    iteration = 0;
    restartPressed = true;  //Set flag to true to begin game 
  }
}

//Routine when action is PULL-IT
bool has_pulled(float time_lim) {
  //Set-up countdown for time player has to pull the antenna
  int duration = (int)(time_lim * 1000);
  int startState = digitalRead(2);
  int threshold = 600; //Tune-It threshold 
  int startVal = analogRead(A0);  //Reads antenna input 

  mp3.setVolume(volume*0.7);  //Cut-out the audio 

  //Begin countdown 
  unsigned long start = millis();
  while ((millis() - start) <= duration) {
    //Check input for if the antenna has been pulled 
    if (digitalRead(2) != startState) {
      mp3.setVolume(volume);
      return true;  //Return success
    } 

    //Check if player has turned the Tune-It instead 
    if (abs(analogRead(A0) - startVal) > threshold) {
      mp3.setVolume(volume);
      return false; //Return fail 
    }
    mp3.loop(); //Keeps audio playing
  }
  mp3.setVolume(volume);
  return false; //Timer ran out - player lost 
}

//Routine when action is TUNE-IT
bool has_tuned(float time_lim) {
  //Set-up countdown for time player has to turn the tuning knob 
  int duration = (int)(time_lim * 1000);
  int threshold = 300;             //Tune-it threshold 
  int startVal = analogRead(A0);   //Tune-its initial input value 
  int startState = digitalRead(2); //Pull-its initial input state 
  int cuts = 0;                    //Audio effects for Tune-it
  int cuttime = 20;

  //Begin countdown and checking inputs 
  unsigned long start = millis();
  while ((millis() - start) <= duration) {
    //Check if player has turned the tuning knob above threshold 
    if (abs(analogRead(A0) - startVal) > threshold) {
      mp3.setVolume(volume);
      return true; //Return success
    }

    //Cut out the audio 
    if(millis()-start > cuts*cuttime){
      mp3.setVolume(volume*(random(3,10)/10.0));
      ++cuts;
    }

    //Check if player has pulled the antenna 
    if (digitalRead(2) != startState) {
      mp3.setVolume(volume);
      return false;  //Return fail 
    } 

    mp3.loop(); //Keeps audio playing 
  }
  mp3.setVolume(volume);
  return false; //Time ran out and player lost 
}

//Routine when action is SHAKE-IT
bool has_shaken(float time_lim) {
  //Set-up countdown and other variables to test if radio has been shaken 
  int duration = (int)(time_lim * 1000);
  float threshold = 1.25;     //Shake-it threshold 
  int cuttime = 200;          //Audio distortion - 200ms between cuts
  int cuts = 0; 
  int tune_threshold = 600;   //Tune-it threshold 
  int startVal = analogRead(A0);  //Tune-its starting input value
  int startState = digitalRead(2);  //Pull-its starting input state 

  //Begin countdown and checking inputs 
  unsigned long start = millis();
  while ((millis() - start) <= duration) {
    //Reading x,y, and z direction accelerations and finding its magnitude 
    float ax = SensorOne.readFloatAccelX();   
    float ay = SensorOne.readFloatAccelY();
    float az = SensorOne.readFloatAccelZ();
    float mag = sqrt(ax * ax + ay * ay + az * az);

    if(millis()-start > cuttime*cuts) {
      mp3.setVolume(volume*round(random(0,2))); //Decrease radio's volume 
      ++cuts;
    }

    //Check if radio was shaken above the threshold
    if (mag > threshold) {
      mp3.setVolume(volume);
      return true; //Return success
    }

    //Check if the antenna has been pulled 
    if (digitalRead(2) != startState) {
      mp3.setVolume(volume);
      return false; //Return fail 
    }  

    //Check is tuning knob has been turned 
    if (abs(analogRead(A0) - startVal) > tune_threshold) {
      mp3.setVolume(volume);
      return false; //Return fail 
    }

    mp3.loop(); //Keeps audio playing 
  }
  return false; //Timer ran out - player loses 
}

//Routine when player loses - input argument = their final score 
void player_lost(byte final_score) {
  oled_display_str(F("You Lost"), 2, 0, true);
  waitMilliseconds(1500);
  oled_display_str(F("Score:"), 2, 0, true);
  waitMilliseconds(500);
  oled_display_score(final_score);
  waitMilliseconds(2000);
  iteration = 101;  //Set iteration above 99 to exit game loop 
}

//Routine when player wins 
void player_won() {
  oled_display_str(F("Congrats!"), 2, 0, true);
  waitMilliseconds(2000);
  oled_display_str(F("You Win!"), 2, 0, true);
  waitMilliseconds(2000);
}

/*Function to display a string to OLED 
  Input arguments - string (storing in flash), size = text size, y = row to put string on, 
  isClear = should oled clear whole display first */
void oled_display_str(const __FlashStringHelper *disp, byte size, byte y, bool isClear) {
  if (isClear){ 
    display.clearDisplay();
  }
  display.setTextSize(size);      //Font size 
  display.setTextColor(WHITE);    
  display.setCursor(0, size*y*12);  //Put cursor to appropriate row 
  display.println(disp);            //Print the string to the oled 
  display.display();      
} 

//Function to display score to OLED - input argument = score 
void oled_display_score(byte score) {
  display.clearDisplay();         //Clear display 
  display.setTextSize(4);         //Font size
  display.setTextColor(WHITE);
  display.setCursor(0, 10);       //Set cursor to top row 
  display.println(score);         //Print the score to the oled 
  display.display();
}

//Function to display a countdown on the OLED - input argument = how many seconds is the timer
void oled_countdown(int seconds) {
  for (int i = seconds; i > 0; i--) { 
    oled_display_score(i);
    waitMilliseconds(1000); //Wait one second before continuing 
  }
}

//Function to delay microcontroller without stopping all other processes - input = delay time in ms 
void waitMilliseconds(uint16_t ms) {
  uint32_t start = millis();
  while ((millis() - start) < ms) {
    mp3.loop();
    //Check if volume has been changed here 
    if(abs(analogRead(A3)/20.5-volume)>5) {
      volume = analogRead(A3)/20.5;
      mp3.setVolume(volume);
    }
    delay(1);
  }
}

//Function to rotate motor 
void rotate_motor(int t_step, int steps) {
  //Pins where motor is connected to 
  int st_a = 5;
  int st_b = 6;
  int st_c = 7;
  int st_d = 8;
  
  //Drive the stepper motor using appropriate pins for each step 
  for(int i = 0; i < steps; ++i) {
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

//Function to put motor back to it's initial position
void motor_reset() {
  //Pins where motor is connected to 
   int st_a = 5;
   int st_b = 6;
   int st_c = 7;
   int st_d = 8;
   int t_step = 2; //Step size 

   //Checking limit switch if motor is back in starting spot
   while(digitalRead(4)==0) {
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
