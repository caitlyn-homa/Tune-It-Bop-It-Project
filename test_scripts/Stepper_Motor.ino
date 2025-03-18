void setup() {
  // put your setup code here, to run once:
  int st_a = 5;
  int st_b = 6;
  int st_c = 7;
  int st_d = 8;

  pinMode(st_a, OUTPUT);
  pinMode(st_b, OUTPUT);
  pinMode(st_c, OUTPUT);
  pinMode(st_d, OUTPUT);
  pinMode(0, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(digitalRead(0) == 0)
  {
    for(int j = 0; j < 90; ++j)
    {
    rotate(1, 2);
    delay(100);
    }
  }
}

void rotate(int t_step, int angle)
{
  int st_a = 5;
  int st_b = 6;
  int st_c = 7;
  int st_d = 8;

  int steps = int(angle*(8.0/5.625));

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