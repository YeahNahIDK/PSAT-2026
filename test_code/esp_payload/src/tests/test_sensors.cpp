#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME680.h>
#include <MPU6050_light.h>
#include "config.h"
#include "tests/test_common.h"

static Adafruit_BME680 bme;
static MPU6050 mpu(Wire);

static void taskBme(void*) {
  TickType_t last = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(200); // 5 Hz

  for (;;) {
    if (bme.performReading()) {
      Serial.print("[BME] T="); Serial.print(bme.temperature);
      Serial.print("C H=");     Serial.print(bme.humidity);
      Serial.print("% P=");     Serial.print(bme.pressure);
      Serial.println("Pa");
    } else {
      Serial.println("[BME] read fail");
    }
    vTaskDelayUntil(&last, period);
  }
}

static void taskMpu(void*) {
  TickType_t last = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(50); // 20 Hz

  for (;;) {
    mpu.update();
    Serial.print("[MPU] ax="); Serial.print(mpu.getAccX(), 3);
    Serial.print(" ay=");      Serial.print(mpu.getAccY(), 3);
    Serial.print(" az=");      Serial.print(mpu.getAccZ(), 3);
    Serial.print(" gx=");      Serial.print(mpu.getGyroX(), 2);
    Serial.print(" gy=");      Serial.print(mpu.getGyroY(), 2);
    Serial.print(" gz=");      Serial.println(mpu.getGyroZ(), 2);
    vTaskDelayUntil(&last, period);
  }
}

void testSensorsStart() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(400000);

  if (!bme.begin()) die("BME680 not found (check CS pulled high for I2C, wiring, addr 0x76/0x77)");
  if (mpu.begin() != 0) die("MPU6050 not found (addr 0x68/0x69, wiring)");

  Serial.println("Keep MPU still: calibrating offsets...");
  mpu.calcOffsets(true, true);
  Serial.println("Sensors OK");

  xTaskCreatePinnedToCore(taskBme, "bme", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(taskMpu, "mpu", 4096, nullptr, 2, nullptr, 0);
}
