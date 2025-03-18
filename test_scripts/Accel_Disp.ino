#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SparkFunLIS3DH.h"
#include "SPI.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
LIS3DH SensorOne( I2C_MODE, 0x19 );

int i = 0;
void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Hello, World!");
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

  pinMode(8, OUTPUT);
}

void loop() {
  float thresh = 1.25;

  float x = SensorOne.readFloatAccelX();
  float y = SensorOne.readFloatAccelY();
  float z = SensorOne.readFloatAccelZ();

  float mag = sqrt(x*x + y*y + z*z);

  if(mag > thresh)
  {
    i++;
    digitalWrite(8, HIGH);

    display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println(i);
  display.display(); 
  }
  else
  {
    digitalWrite(8, LOW);
  }

  delay(10);
}