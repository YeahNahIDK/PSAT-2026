#pragma once
#include <Arduino.h>

// Data structure for storing IMU samples
struct ImuSample {
  float t;   // time (s)
  float ax, ay, az; // acceleration (g)
  float gx, gy, gz; // gyro (dps)
};

// --- Public API ---

// Configure SDA/SCL pins (optional, must call before initIMU()).
void configureIMUPins(uint8_t sda, uint8_t scl);

// Initialise the IMU (I2C, start accel+gyro).
// Returns true if IMU found and configured.
bool initIMU();

// Poll IMU and push new data into buffer.
void serviceIMU();

// Pop the oldest buffered sample into `s`. Returns false if no new data.
bool readIMU(ImuSample &s);

