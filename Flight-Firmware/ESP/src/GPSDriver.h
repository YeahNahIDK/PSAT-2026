#ifndef GPS_DRIVER_H
#define GPS_DRIVER_H

#include <Arduino.h>
#include <TinyGPS++.h>

class GPSDriver {
public:
    // Constructor: accepts UART number, pins, and baud rate
    GPSDriver(int uartNr, int rxPin, int txPin, long baudRate = 9600);

    // Initializer
    bool begin();

    // Call this repeatedly in the main loop
    // Returns true if a new valid sentence was received
    bool update();

    // Getters for common data
    double getLatitude();
    double getLongitude();
    double getAltitude();
    int getSatellites();
    bool isValid(); // Do we have a fix?

    // Access to raw TinyGPS object if needing specific fields (time, date, etc)
    TinyGPSPlus tGps; 

private:
    HardwareSerial _serial;
    int _rxPin;
    int _txPin;
    long _baudRate;
};

#endif