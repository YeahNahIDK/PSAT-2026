#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "config.h"
#include "tests/test_common.h"

static SPIClass spiSD(FSPI);

static void taskSd(void*) {
  TickType_t last = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(500); // 2 Hz

  for (;;) {
    File f = SD.open("/sd_test.csv", FILE_APPEND);
    if (f) {
      f.print(millis());
      f.print(",hello\n");
      f.close();
      Serial.println("[SD] wrote a line to /sd_test.csv");
    } else {
      Serial.println("[SD] open failed");
    }
    vTaskDelayUntil(&last, period);
  }
}

void testSdStart() {
  spiSD.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS, spiSD)) die("SD mount failed (power/CS/SPI pins/FAT32)");

  Serial.println("[SD] mounted OK");
  xTaskCreatePinnedToCore(taskSd, "sd", 4096, nullptr, 1, nullptr, 0);
}
