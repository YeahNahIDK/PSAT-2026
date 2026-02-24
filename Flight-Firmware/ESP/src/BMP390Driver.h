#ifndef BMP390_DRIVER_H
#define BMP390_DRIVER_H

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"

class BMP390Driver {
public:
    BMP390Driver();
    bool begin();
    
    void calibrate(); 
    
    float getAltitude();    // Returns meters (AGL)
    float getPressure();    // Returns hPa
    float getTemperature(); // Returns C

private:
    Adafruit_BMP3XX bmp;
    const float SEALEVELPRESSURE_HPA = 1013.25; 
    float _groundOffset = 0.0; 
};

#endif