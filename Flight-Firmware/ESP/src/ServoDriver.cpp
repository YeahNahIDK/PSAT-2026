#include "ServoDriver.h"
#include "hardware_config.h"

#define SERVO_FREQUENCY         50
#define SERVO_TIMER_RESOLUTION  14
#define SERVO_TIMER_CHANNEL     0

ServoDriver::ServoDriver(int pin, uint32_t minUs, uint32_t maxUs) 
    : _servoPin(pin), _minUs(minUs), _maxUs(maxUs) {}

void ServoDriver::begin() {
    ledcAttachChannel(_servoPin, SERVO_FREQUENCY, SERVO_TIMER_RESOLUTION, SERVO_TIMER_CHANNEL);
}

void ServoDriver::writeMicroseconds(uint32_t us) {
    if (us < _minUs) us = _minUs;
    if (us > _maxUs) us = _maxUs;
    
    uint32_t duty = (us * 16384) / 20000; 
    ledcWrite(_servoPin, duty);
}

void ServoDriver::writeAngle(float angle) {
    if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
    if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

    // Calculate the percentage of travel regardless of the starting angle
    float angleRange = SERVO_MAX_ANGLE - SERVO_MIN_ANGLE;
    float progress = (angle - SERVO_MIN_ANGLE) / angleRange;
    
    // Apply that percentage to the microsecond range
    uint32_t us = _minUs + (uint32_t)(progress * (_maxUs - _minUs));
    
    writeMicroseconds(us);
}