#pragma once
#include <Arduino.h>

class ServoDriver {
private:
    int _servoPin;
    uint32_t _minUs;
    uint32_t _maxUs;

public:
    // Defaulting to 500us (0°) and 2500us (180°)
    ServoDriver(int pin, uint32_t minUs = 500, uint32_t maxUs = 2500);
    
    void begin();

    void writeMicroseconds(uint32_t us); 
    
    void writeAngle(float angle); 
};