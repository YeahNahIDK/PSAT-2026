#ifndef GPS_DRIVER_H
#define GPS_DRIVER_H

#include <Arduino.h>
#include <TinyGPS++.h>

class GPSDriver {
public:
    GPSDriver(int uartNr, int rxPin, int txPin, long baudRate = 115200);
    bool begin();

    bool update();

    double getLatitude();
    double getLongitude();
    double getAltitude();
    int getSatellites();
    bool isValid();
    int getHour();
    int getMinute();
    int getSecond();

    TinyGPSPlus tGps; 

private:
    HardwareSerial _serial;
    int _rxPin;
    int _txPin;
    long _baudRate;
};

#endif