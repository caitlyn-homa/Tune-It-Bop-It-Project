// Optimized Bop-It Game Code for ATMega328 (Memory-efficient)
#include <DFMiniMp3.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include "SparkFunLIS3DH.h" 
#include "Mp3Manager.hpp"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

LIS3DH SensorOne(I2C_MODE, 0x19);
Mp3Manager mp3;

// Game state
byte score = 0;

//Timing Parameters
int numRounds = 99;
//float start_time = 4.0;
//float end_time = 0.7;
float time_constant = -numRounds/log(0.7/4);
float time_to_complete = 4.0;

int volume = analogRead(A3)/20.5;
bool isComplete = false;
byte iteration = 0;
volatile bool restartPressed = false;

// Memory check (debugging only)
extern int __heap_start, *__brkval;
int freeMemory() {
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void setup() {
  pinMode(3, INPUT_PULLUP); // Start button
  attachInterrupt(digitalPinToInterrupt(3), restart_game, FALLING);
  pinMode(4, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP); // Pull sensor
  pinMode(A0, INPUT);        // Potentiometer

  // Stepper motor pins
  for (int i = 5; i <= 8; i++) pinMode(i, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  SensorOne.begin();
}

void loop() {
  waitMilliseconds(100);
  //oled_display_str(F("Press Start"), 2);
  oled_display_str(F("Press"), 2, 0, true);
  oled_display_str(F("Start"), 2, 1, false);

  if (restartPressed) {
    restartPressed = false;
    mp3.begin();
    waitMilliseconds(10);
    motor_reset();
    oled_display_str(F("Get Ready!"), 2, 0, true);
    oled_countdown(3);

    for (iteration = 0; iteration < numRounds; iteration++) {
      mp3.loop();
      isComplete = false;
      byte action_number = random(1, 4);

      if (action_number == 1) {
        oled_display_str(F("Pull It!"), 2, 0, true);
        waitMilliseconds(300);
        isComplete = has_pulled(time_to_complete);
      } else if (action_number == 2) {
        oled_display_str(F("Tune It!"), 2, 0, true);
        waitMilliseconds(300);
        isComplete = has_tuned(time_to_complete);
      } else {
        oled_display_str(F("Shake It!"), 2, 0, true);
        waitMilliseconds(300);
        isComplete = has_shaken(time_to_complete);
      }

      if (isComplete) {
        score++;
        mp3.setVolume(volume);
        oled_display_str(F("Signal"), 2, 0, true);
        oled_display_str(F("Connected!"), 2, 1, false);
        rotate_motor(1, 2);
        waitMilliseconds(600);
      } else {
        player_lost(score);
        break;
      }

      //Update game time
      time_to_complete = 4 * exp(-(iteration+1)/time_constant);
    }

    if (score == numRounds) player_won();
    iteration = 0;
  }
}

void restart_game() {
  static unsigned long lastInterruptTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > 300) {
    lastInterruptTime = currentTime;
    score = 0;
    time_to_complete = 4;
    iteration = 0;
    restartPressed = true;
  }
}

bool has_shaken(float time_lim) {
  int duration = (int)(time_lim * 1000);
  float threshold = 1.25;
  unsigned long start = millis();
  int cuttime = 200; //200ms between cuts
  int tune_threshold = 600;
  int startVal = analogRead(A0);
  int cuts = 0;
  int startState = digitalRead(2);

  while ((millis() - start) <= duration) {
    float ax = SensorOne.readFloatAccelX();
    float ay = SensorOne.readFloatAccelY();
    float az = SensorOne.readFloatAccelZ();
    float mag = sqrt(ax * ax + ay * ay + az * az);

    if(millis()-start > cuttime*cuts)
    {
      mp3.setVolume(volume*round(random(0,2)));
      ++cuts;
    }

    //Succeed if shaken
    if (mag > threshold)
    {
      mp3.setVolume(volume);
      return true;
    }

    //Fail if pulled
    if (digitalRead(2) != startState) 
    {
      mp3.setVolume(volume);
      return false;
    }  

    //Fail if tuned
    if (abs(analogRead(A0) - startVal) > tune_threshold) 
    {
      mp3.setVolume(volume);
      return false;
    }

    mp3.loop();
  }
  return false;
}

bool has_pulled(float time_lim) {
  int duration = (int)(time_lim * 1000);
  int startState = digitalRead(2);
  int threshold = 600;
  int startVal = analogRead(A0);
  unsigned long start = millis();

  mp3.setVolume(volume*0.7);

  while ((millis() - start) <= duration) {

    //Succeed if pulled
    if (digitalRead(2) != startState) 
    {
      mp3.setVolume(volume);
      return true;
    } 

    //Fail if tuned
    if (abs(analogRead(A0) - startVal) > threshold) 
    {
      mp3.setVolume(volume);
      return false;
    }

    mp3.loop();
  }
  mp3.setVolume(volume);
  return false;
}

bool has_tuned(float time_lim) {
  int duration = (int)(time_lim * 1000);
  int threshold = 300;
  int startVal = analogRead(A0);
  unsigned long start = millis();
  int startState = digitalRead(2);
  int cuts = 0;
  int cuttime = 20;

  while ((millis() - start) <= duration) {
    //Succeed if tuned
    if (abs(analogRead(A0) - startVal) > threshold) 
    {
      mp3.setVolume(volume);
      return true;
    }

    if(millis()-start > cuts*cuttime)
    {
      mp3.setVolume(volume*(random(3,10)/10.0));
      ++cuts;
    }

    //Fail if pulled
    if (digitalRead(2) != startState) 
    {
      mp3.setVolume(volume);
      return false;
    } 

    mp3.loop();
  }
  mp3.setVolume(volume);
  return false;
}

void player_lost(byte final_score) {
  oled_display_str(F("You Lost"), 2, 0, true);
  waitMilliseconds(1500);
  oled_display_str(F("Score:"), 2, 0, true);
  waitMilliseconds(500);
  oled_display_score(final_score);
  waitMilliseconds(2000);
  iteration = 101;
}

void player_won() {
  oled_display_str(F("Congrats!"), 2, 0, true);
  waitMilliseconds(2000);
  oled_display_str(F("You Win!"), 2, 0, true);
  waitMilliseconds(2000);
}

// void oled_display_str(const __FlashStringHelper *disp, byte size) {
//   display.clearDisplay();
//   display.setTextSize(size);
//   display.setTextColor(WHITE);
//   display.setCursor(0, 10);
//   display.println(disp);
//   display.display();
// }

void oled_display_str(const __FlashStringHelper *disp, byte size, byte y, bool isClear) {
  if (isClear){ 
    display.clearDisplay();
  }
  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(0, size*y*12);    //size = font size, y is want number row 
  display.println(disp);
  display.display();
}

void oled_display_score(byte score) {
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println(score);
  display.display();
}

void oled_countdown(int seconds) {
  for (int i = seconds; i > 0; i--) {
    oled_display_score(i);
    waitMilliseconds(1000);
  }
}

void waitMilliseconds(uint16_t ms) {
  uint32_t start = millis();
  while ((millis() - start) < ms) {
    mp3.loop();
    if(abs(analogRead(A3)/20.5-volume)>5)
    {
      volume = analogRead(A3)/20.5;
      mp3.setVolume(volume);
    }
    delay(1);
  }
}

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
