// gps.h — GPS READING AND NMEA PARSING (PSEUDOCODE)

// This module handles all communication with the Adafruit Ultimate GPS.
// The GPS sends ASCII text lines called NMEA sentences over UART.
// Example:
//   $GPGGA,....\n


void gps_init();             // Setup UART + clear buffers
void gps_read();             // Called frequently: collects serial bytes
int  gps_has_fix();          // 1 if GPS lock is valid
float gps_lat();             // Latitude in degrees
float gps_lon();             // Longitude in degrees
float gps_alt();             // Altitude as reported by GPS (meters)
