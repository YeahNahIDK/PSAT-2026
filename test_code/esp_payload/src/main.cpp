#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "app/app_state.h"
#include "app/tasks.h"

/*void setup() {
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
*/

// test selector 

#include <Arduino.h>
#include "config.h"

// Each test provides a start function
void testServoUartStart();
void testLoRaUartStart();
void testSensorsStart();
void testSdStart();
void testAllStart();

void setup() {
  Serial.begin(BAUD_DEBUG);
  vTaskDelay(pdMS_TO_TICKS(200));
  Serial.println("\n=== PSAT Payload Test Harness ===");

#if defined(TEST_MODE_SERVO_UART)
  Serial.println("Boot mode: TEST_MODE_SERVO_UART");
  testServoUartStart();

#elif defined(TEST_MODE_LORA_UART)
  Serial.println("Boot mode: TEST_MODE_LORA_UART");
  testLoRaUartStart();

#elif defined(TEST_MODE_SENSORS)
  Serial.println("Boot mode: TEST_MODE_SENSORS");
  testSensorsStart();

#elif defined(TEST_MODE_SD)
  Serial.println("Boot mode: TEST_MODE_SD");
  testSdStart();

#elif defined(TEST_MODE_ALL)
  Serial.println("Boot mode: TEST_MODE_ALL");
  testAllStart();

#else
  Serial.println("ERROR: No TEST_MODE_* defined in platformio.ini");
#endif
}

void loop() {
  // Intentionally empty: tests run via FreeRTOS tasks.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
