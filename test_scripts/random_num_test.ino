//Test if random number generation is working through outputs and LEDs 

//Define random number varible
byte rand_num = 0;

void setup() {
  //Initialize communication with computer from microprocessor 
  Serial.begin(9600);

  //Seed random number generation 
  randomSeed(analogRead(0));

  //Set digital pins 8,7,6 as outputs for LEDs 
  pinMode(8, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);

}

void loop() {
  //Generate random number between 1 and 3
  rand_num = random(1,4);

  //if number is 1 - turn digital pin 6 on and off
  if(rand_num == 1){
    Serial.println("1");
    digitalWrite(6, HIGH);  
    delay(1000);                      
    digitalWrite(6, LOW);   
    delay(1000); 
  }

  //if number is 2 - turn digital pin 7 on and off
  if(rand_num == 2){
    Serial.println("2");
    digitalWrite(7, HIGH);  
    delay(1000);                      
    digitalWrite(7, LOW);   
    delay(1000); 
  }
  //if number is 3 - turn digital pin 8 on and off
  if(rand_num == 3){
    Serial.println("3");
    digitalWrite(8, HIGH);  
    delay(1000);                      
    digitalWrite(8, LOW);   
    delay(1000); 
  }

}
