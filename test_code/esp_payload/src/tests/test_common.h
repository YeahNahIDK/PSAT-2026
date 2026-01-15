#pragma once
#include <Arduino.h>

static inline void die(const char* msg) {
  Serial.println(msg);
  for (;;) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}
