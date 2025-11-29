const uint8_t rxPin = 12;
const uint8_t txPin = 13;

void setup() {
  Serial.begin(115200);
  
  Serial1.begin(9600, SERIAL_8N1, rxPin, txPin);
  
}

void loop() {
  // Forward bytes from USB Serial to RA-08H
  while (Serial.available()) {
    Serial1.write(Serial.read());
  }

  // Forward response from RA-08H back to USB
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}
