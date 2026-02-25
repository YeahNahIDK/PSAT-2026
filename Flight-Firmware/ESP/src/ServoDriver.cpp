#include "ServoDriver.h"
#include "hardware_config.h"

ServoDriver::ServoDriver(int pin, uint32_t minUs, uint32_t maxUs) 
    : _servoPin(pin), _minUs(minUs), _maxUs(maxUs) {}

void ServoDriver::begin() {
    // Attach the pin to an auto-assigned timer at 50Hz with 14-bit resolution
    ledcAttach(_servoPin, 50, 14);
}

void ServoDriver::writeMicroseconds(uint32_t us) {
    if (us < _minUs) us = _minUs;
    if (us > _maxUs) us = _maxUs;
    
    uint32_t duty = (us * 16384) / 20000; 
    ledcWrite(_servoPin, duty);
}

void ServoDriver::writeAngle(float angle) {
    if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
    if (angle > SERVO_MAX_ANGLE ) angle = SERVO_MAX_ANGLE ;

    // Map the 0-180 degree range to your min and max microseconds
    uint32_t us = _minUs + (uint32_t)((angle / SERVO_MAX_ANGLE) * (_maxUs - _minUs));
    
    writeMicroseconds(us);
}