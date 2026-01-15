#pragma once
#include <Arduino.h>
#include "sensors/bme680_sensor.h"
#include "sensors/mpu6050_sensor.h"

// This struct holds the latest sensor values.
// Multiple tasks access it, so we protect it with a mutex.
struct SharedSensors {
  BMEData bme;
  IMUData imu;
};

// Global shared sensor snapshot + mutex
extern SharedSensors gSensors;
extern SemaphoreHandle_t gSensorsMutex;
