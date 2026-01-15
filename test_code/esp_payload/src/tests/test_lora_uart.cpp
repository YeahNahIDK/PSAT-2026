#include <Arduino.h>
#include "config.h"

static HardwareSerial LinkLoRa(2);

static void taskLoRaUart(void*) {
  TickType_t last = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(100); // 10 Hz

  for (;;) {
    const uint32_t t = millis();

    // Minimal payload: time + dummy altitude
    float alt = 123.45f;

    char line[64];
    int n = snprintf(line, sizeof(line), "T,%lu,%.2f\n", (unsigned long)t, alt);

    LinkLoRa.write((uint8_t*)line, (size_t)n);
    Serial.print("[LORA_UART] "); Serial.print(line);

    vTaskDelayUntil(&last, period);
  }
}

void testLoRaUartStart() {
  LinkLoRa.begin(BAUD_LINK, SERIAL_8N1, PIN_MSP_LORA_RX, PIN_MSP_LORA_TX);
  xTaskCreatePinnedToCore(taskLoRaUart, "lora_uart", 4096, nullptr, 2, nullptr, 0);
}
