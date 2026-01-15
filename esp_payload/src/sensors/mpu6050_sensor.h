#pragma once
#include <Arduino.h>

struct IMUData {
  // From MPU6050_light:
  // Accel is in "g" units, gyro is in deg/s.
  float ax=0, ay=0, az=0;
  float gx=0, gy=0, gz=0;

  // Roll/pitch from library’s internal filter (yaw not reliable without magnetometer)
  float roll=0, pitch=0;

  // Magnitude of acceleration vector (useful for flight detection)
  float aMagG=0;
};

class MPU6050Sensor {
public:
  bool begin();
  bool update(IMUData &out);

private:
  bool _ok = false;
};
