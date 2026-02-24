#pragma once
#include <Arduino.h>
#include <Wire.h>

// Data container
struct IMUData {
    float accX, accY, accZ; // g
    float gyrX, gyrY, gyrZ; // dps
};

class ICMDriver {
public:
    ICMDriver(TwoWire &wirePort = Wire, uint8_t addr = 0x68);
    
    bool begin();
    
    bool update();
    
    IMUData getData() const;

    void calibrateGyro();

private:
    TwoWire* _wire;
    uint8_t _addr;
    IMUData _data;
    
    float _gyroOffsetX = 0.0f;
    float _gyroOffsetY = 0.0f;
    float _gyroOffsetZ = 0.0f;

    void writeRegister(uint8_t reg, uint8_t data);
    uint8_t readRegister(uint8_t reg);
};