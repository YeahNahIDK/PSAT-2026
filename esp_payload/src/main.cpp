#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "app/app_state.h"
#include "app/tasks.h"

void setup() {
  Serial.begin(115200);

  // ---------- I2C ----------
  // We initialise Wire once here, then tasks use it.
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(400000);

  // ---------- Shared state protection ----------
  gSensorsMutex = xSemaphoreCreateMutex();

  // ---------- Queues ----------
  gLogQueue = xQueueCreate(LOG_QUEUE_LEN, sizeof(LogRow));
  gCmdQueue = xQueueCreate(CMD_QUEUE_LEN, sizeof(SDCmd));

  // ---------- Tasks ----------
  // Priorities: IMU highest (timing sensitive), SD lowest (can stall sometimes)
  xTaskCreatePinnedToCore(taskIMU,    "IMU",    4096, nullptr, 3, nullptr, 0);
  xTaskCreatePinnedToCore(taskBME,    "BME",    4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(taskFlight, "FLIGHT", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(taskLog,    "LOG",    4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(taskSD,     "SD",     8192, nullptr, 1, nullptr, 0);
}

void loop() {
  // Intentionally empty.
  // FreeRTOS tasks run everything.
}
