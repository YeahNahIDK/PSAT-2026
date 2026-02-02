#include "imu.h"
#include <Wire.h>
#include <ICM42670P.h>

// --- Default I2C pins depending on target MCU ---
#if CONFIG_IDF_TARGET_ESP32C3
  #define DEFAULT_IMU_SDA 8
  #define DEFAULT_IMU_SCL 9
#else
  #define DEFAULT_IMU_SDA 21
  #define DEFAULT_IMU_SCL 22
#endif

// --- Module state ---
static ICM42670 imu(Wire, false);  // I2C, addr 0x68
static uint8_t imu_sda = DEFAULT_IMU_SDA;
static uint8_t imu_scl = DEFAULT_IMU_SCL;

// Circular buffer for samples
const int BUF_SIZE = 32;
static ImuSample buf[BUF_SIZE];
static int head = 0, tail = 0;

// --- Public API ---

void configureIMUPins(uint8_t sda, uint8_t scl) {
  imu_sda = sda;
  imu_scl = scl;
}

bool initIMU() {
  Wire.begin(imu_sda, imu_scl);
  delay(100);

  if (imu.begin() != 0) {
    Serial.println("[IMU] ERROR: Failed to find ICM42670 chip");
    return false;
  }

  imu.startAccel(104, 4);    // 104 Hz, ±4 g
  imu.startGyro(104, 250);   // 104 Hz, ±250 dps

  Serial.println("[IMU] Initialised OK");
  return true;
}

void serviceIMU() {
  static int warmup = 20;  // discard first 20 samples
  inv_imu_sensor_event_t evt;

  if (imu.getDataFromRegisters(evt) == 0) {

    // ---- Warm-up discard ----
    if (warmup > 0) {
      warmup--;
      return;   // skip writing to buffer
    }

    // ---- Scale factors ----
    const float ACCEL_SCALE = 1.0f / 8192.0f;  // ±4 g
    const float GYRO_SCALE  = 1.0f / 131.0f;   // ±250 dps

    float ax =  evt.accel[0] * ACCEL_SCALE;
    float ay = -evt.accel[1] * ACCEL_SCALE;
    float az =  evt.accel[2] * ACCEL_SCALE;

    float gx =  evt.gyro[0] * GYRO_SCALE;
    float gy = -evt.gyro[1] * GYRO_SCALE;
    float gz =  evt.gyro[2] * GYRO_SCALE;

    // ---- Junk filter ----
    // If accel magnitude ≈ 0 g *and* gyro values are crazy → drop sample
    float accelMag = sqrtf(ax*ax + ay*ay + az*az);
    if ((accelMag < 0.2f) && (fabs(gx) > 100.0f || fabs(gy) > 100.0f || fabs(gz) > 100.0f)) {
      return;  // skip junk sample
    }

    // ---- Store valid sample in buffer ----
    head = (head + 1) % BUF_SIZE;
    buf[head].t = millis() / 1000.0f;
    buf[head].ax = ax;
    buf[head].ay = ay;
    buf[head].az = az;
    buf[head].gx = gx;
    buf[head].gy = gy;
    buf[head].gz = gz;
  }
}


bool readIMU(ImuSample &s) {
  if (tail == head) return false;
  tail = (tail + 1) % BUF_SIZE;
  s = buf[tail];
  return true;
}
