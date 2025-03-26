#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>

//Keep track of player's score throughout the game
int score = 0;
//Initial time player has to complete a task [seconds]
int time_to_complete = 3;

void setup() {
  //Seed random number generation with current time -ensures variation in the sequence of random numbers 
  randomSeed(millis());

  //Pin Set-Up 
  // Reset/Start button
  pinMode(8, OUTPUT);
  pinMode(3, INPUT_PULLUP);
  // Pull Task
  pinMode(8, OUTPUT);
  pinMode(1, INPUT_PULLUP);
  // Tune Task
  pinMode(8, OUTPUT);
  // Shake Task
  pinMode(8, OUTPUT);
  // Stepper Motor 
  pinMode(5, OUTPUT); //st_a
  pinMode(6, OUTPUT); //st_b
  pinMode(7, OUTPUT); //st_c
  pinMode(8, OUTPUT); //st_d
  //pinMode(0, INPUT_PULLUP);
  // OLED Display
  pinMode(8, OUTPUT);  
  // Audio player 

  //OLED set-up - uncomment the function - i don't have library installed for it :(
  //oled_set_up();

}

void loop() {
  
  //If start button has been pushed - begin the game 
  if (digitalRead(3) == LOW){
    
    //Generates random number 1 through 3 to select action - MAP -> #1 = Pull-it #2 = Tune-it #3 = Shake-it
    byte action_number = random(1,4);

  }

}

//Returns true or false depending if player has succesfully shaken the radio
//Parameter - time_lime = time player has to complete the task 
bool has_shaken(int time_lim)
{
  //Convert time limit to milliseconds
  time_lim = time_lim * 1000

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
    if(mag > thresh)
    {
      return true 
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
  time_lim = time_lim * 1000

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
  time_lim = time_lim * 1000

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

//Rotates motor needs specific time step and angle 
void rotate_motor(int t_step, int angle)
{
  //Pins where motor is set up at 
  int st_a = 5;
  int st_b = 6;
  int st_c = 7;
  int st_d = 8;

  //Calculate number of steps needed to rotate for specified angle
  int steps = int(angle*(8.0/5.625));
  
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

/*
//Sets up the OLED and displays Tune It! message 
void oled_set_up(){

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  delay(2000);
  display.clearDisplay();

  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Tune It!");
  display.display(); 

  Serial.begin(9600);
  delay(1000); //relax...
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
*/

