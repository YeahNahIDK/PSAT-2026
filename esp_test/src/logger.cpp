#include "logger.h"
#include <Arduino.h>
#include <SD.h>
#include "config.h"

File dataFile;
File loraFile;
static int currentFlight = 0;

static int nextFlightNumber() {
  int n = 1;
  char filename[20];
  while (true) {
    snprintf(filename, sizeof(filename), "/flight%d.csv", n);
    if (!SD.exists(filename)) break;
    n++;
  }
  return n;
}

void initLogger() {
  if (!SD.begin(PIN_SD_CS)) {
    Serial.println("SD init failed!");
    return;
  }

  currentFlight = nextFlightNumber();

  char filename[20];
  snprintf(filename, sizeof(filename), "/flight%d.csv", currentFlight);

  dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {

    // Updated header for BME680 + IMU
    dataFile.println(
      "Timestamp_ms,"
      "Pressure_hPa,RelAltitude_m,"
      "Temp_C,Humidity_RH,Gas_Ohms,"
      "Ax,Ay,Az,Gx,Gy,Gz"
    );
    dataFile.flush();

    Serial.printf("[SD] Logging started: %s\n", filename);

    File summary = SD.open("/flights.csv", FILE_APPEND);
    if (summary) {
      summary.printf("Flight %d started\n", currentFlight);
      summary.close();
    }
  } else {
    Serial.println("Failed to open log file");
  }

  // LoRa log
  loraFile = SD.open("/lora_log.csv", FILE_APPEND);
  if (loraFile) {
    Serial.println("[SD] LoRa log ready: /lora_log.csv");
  } else {
    Serial.println("Failed to open LoRa log file!");
  }
}

void logSensors(unsigned long t_ms,
                float pressure_hPa, float relAlt_m,
                float temp_C, float humidity_RH, float gas_Ohms,
                float ax, float ay, float az,
                float gx, float gy, float gz) {
  if (!dataFile) return;

  dataFile.printf(
    "%lu,%.2f,%.2f,%.2f,%.2f,%.0f,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f\n",
    t_ms,
    pressure_hPa, relAlt_m,
    temp_C, humidity_RH, gas_Ohms,
    ax, ay, az,
    gx, gy, gz
  );
  dataFile.flush();

#if DEBUG_MODE
  Serial.printf(
    "[%lu ms] P=%.2f hPa RelAlt=%.2f m | "
    "T=%.2f C RH=%.2f%% Gas=%.0fΩ | "
    "A[g]=%.3f %.3f %.3f | G[dps]=%.2f %.2f %.2f\n",
    t_ms,
    pressure_hPa, relAlt_m,
    temp_C, humidity_RH, gas_Ohms,
    ax, ay, az,
    gx, gy, gz
  );
#endif
}

void logLoraData(const String &line) {
  if (!loraFile) return;
  loraFile.println(line);
  loraFile.flush();
}

void closeLogger() {
  if (dataFile) {
    dataFile.flush();
    dataFile.close();
    Serial.println("Logger closed");
  }

  if (loraFile) {
    loraFile.flush();
    loraFile.close();
    Serial.println("LoRa log closed");
  }
}

int getFlightCount() {
  int lines = 0;
  File summaryFile = SD.open("/flights.csv");
  if (summaryFile) {
    while (summaryFile.available()) {
      if (summaryFile.read() == '\n') lines++;
    }
    summaryFile.close();
  }
  return lines;
}

int getCurrentFlightNumber() {
  return currentFlight;
}
