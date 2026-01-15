#include "mpu6050_sensor.h"
#include <Wire.h>
#include <MPU6050_light.h>

// Library object
static MPU6050 mpu(Wire);

bool MPU6050Sensor::begin() {
  // begin() returns 0 on success (library convention)
  byte status = mpu.begin();
  if (status != 0) return false;

  // Calibrate offsets (device must be still)
  // This is NOT delay(); it's just math and I2C reads internally.
  mpu.calcOffsets(true, true);

  _ok = true;
  return true;
}

bool MPU6050Sensor::update(IMUData &out) {
  if (!_ok) return false;

  // Update internal state
  mpu.update();

  // Copy readings
  out.ax = mpu.getAccX();
  out.ay = mpu.getAccY();
  out.az = mpu.getAccZ();

  out.gx = mpu.getGyroX();
  out.gy = mpu.getGyroY();
  out.gz = mpu.getGyroZ();

  out.roll  = mpu.getAngleX();
  out.pitch = mpu.getAngleY();

  // Accel magnitude (g)
  out.aMagG = sqrtf(out.ax*out.ax + out.ay*out.ay + out.az*out.az);
  return true;
}
