#pragma once
#include <Arduino.h>
#include "hardware_config.h"

class ServoDriver {
private:
    int _servoPin;
    uint32_t _minUs;
    uint32_t _maxUs;

public:
    ServoDriver(int pin, uint32_t minUs = SERVO_MIN_ANGLE_US, uint32_t maxUs = SERVO_MAX_ANGLE_US);
    
    void begin();

    void writeMicroseconds(uint32_t us); 
    
    void writeAngle(float angle); 
};