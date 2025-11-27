#include <Adafruit_GPS.h>
#include <SoftwareSerial.h> // For Arduino UNO

const uint8_t txPin = 3;
const uint8_t rxPin = 2;
SoftwareSerial gpsSerial(rxPin, txPin); // For Arduino UNO
Adafruit_GPS GPS(&gpsSerial);

void setup() {
  Serial.begin(115200);

  GPS.begin(9600); // Default baud rate is 9600
  GPS.sendCommand("$PGCMD, 33, 0,*6d"); // Disable antenna warning
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