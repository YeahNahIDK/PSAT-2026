#include <Adafruit_GPS.h>

const uint8_t rxPin = 12;
const uint8_t txPin = 13;

Adafruit_GPS GPS(&Serial1);

void setup() {
  Serial.begin(115200);

  Serial1.begin(9600, SERIAL_8N1, rxPin, txPin);
  GPS.begin(9600); // Default baud rate is 9600
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); // Only output RMC and GGA
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);

  delay(1000); // Let the commands get set
}

void loop() {
  GPS.read();

  if (GPS.newNMEAreceived()) {
    GPS.parse(GPS.lastNMEA());
    Serial.println(GPS.lastNMEA());
  }
}