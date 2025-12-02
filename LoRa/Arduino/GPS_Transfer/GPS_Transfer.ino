#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

// Pin definitions
#define LORA_TX 13
#define LORA_RX 12
#define GPS_TX  4
#define GPS_RX  5

// UART definition
SoftwareSerial GPSSerial(GPS_RX, GPS_TX);
#define LoRaSerial Serial1

Adafruit_GPS GPS(&GPSSerial);

void setup() {
  Serial.begin(115200);

  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  GPSSerial.begin(9600, SWSERIAL_8N1, GPS_RX, GPS_TX);
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); // Only output RMC and GGA
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  delay(1000); // Let the commands get set
}

void loop() {
  GPS.read();

  if (GPS.newNMEAreceived()) {
    GPS.parse(GPS.lastNMEA());
    LoRaSerial.println(GPS.lastNMEA());
  }

  if (Serial.available()) {
    LoRaSerial.write(Serial.read());
  }

  if (LoRaSerial.available()) {
    Serial.write(LoRaSerial.read());
  }
}