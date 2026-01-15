#include <Arduino.h>

// Reuse the individual test start functions
void testSensorsStart();
void testSdStart();
void testLoRaUartStart();

void testAllStart() {
  testSensorsStart();
  testSdStart();
  testLoRaUartStart();

  Serial.println("ALL tests started: sensors + SD + LoRa UART");
}
