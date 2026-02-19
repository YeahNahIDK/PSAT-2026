#pragma once
#include <Arduino.h>

class ServoDriver {
private:
    int _servoPin;
    uint32_t _minUs;
    uint32_t _maxUs;

public:
    // Constructor now includes optional min and max microsecond bounds
    // Defaulting to 500us (0°) and 2500us (180°)
    ServoDriver(int pin, uint32_t minUs = 500, uint32_t maxUs = 2500);
    
    void begin();
    
    // Low-level hardware control
    void writeMicroseconds(uint32_t us); 
    
    // High-level abstraction
    void writeAngle(float angle); 
};