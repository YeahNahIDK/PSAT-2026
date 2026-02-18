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
    // addr is usually 0x68 (default) or 0x69
    ICMDriver(TwoWire &wirePort = Wire, uint8_t addr = 0x68);
    
    bool begin();
    
    // Reads data. Returns true if successful.
    bool update();
    
    IMUData getData() const;

private:
    TwoWire* _wire;
    uint8_t _addr;
    IMUData _data;

    void writeRegister(uint8_t reg, uint8_t data);
    uint8_t readRegister(uint8_t reg);
};