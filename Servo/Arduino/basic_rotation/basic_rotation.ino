void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(7, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(7, HIGH);
  delayMicroseconds(500);
  digitalWrite(7, LOW);
  
  delay(1000);

  digitalWrite(7, HIGH);
  delayMicroseconds(1500);
  digitalWrite(7, LOW);

  delay(1000);
}
