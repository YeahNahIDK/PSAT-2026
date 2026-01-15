#pragma once
#include <Arduino.h>

// ===================== BOARD PINS =====================
// ESP32-C3 Super Mini (from your pinout image)
static const int PIN_I2C_SDA = 8;
static const int PIN_I2C_SCL = 9;

// microSD SPI pins 
static const int PIN_SD_SCK  = 4;
static const int PIN_SD_MISO = 5;
static const int PIN_SD_MOSI = 6;
static const int PIN_SD_CS   = 7;

// ===================== RATES =====================

static const uint32_t IMU_HZ = 50;   // IMU read rate
static const uint32_t BME_HZ = 10;   // BME read rate (pressure/alt slower is fine)
static const uint32_t LOG_HZ = 50;   // Log rate (match IMU)

// ===================== FLIGHT DETECTION =====================
// tune values
static const float LIFTOFF_G_THRESHOLD = 1.8f;   // g
static const float MIN_ALT_RISE_M      = 10.0f;  // meters above baseline before we accept liftoff
static const float LAND_G_THRESHOLD    = 1.2f;   // g (near 1g)
static const float LAND_ALT_WINDOW_M   = 5.0f;   // meters around baseline to consider "ground-ish"
static const uint32_t LIFTOFF_HOLD_MS  = 120;    // must stay above threshold this long
static const uint32_t LAND_HOLD_MS     = 2000;   // must look landed this long

// ===================== SD LOGGING =====================
static const uint32_t LOG_QUEUE_LEN    = 256;    // number of rows buffered in RAM
static const uint32_t CMD_QUEUE_LEN    = 8;      // flight start/stop commands
static const uint32_t FLUSH_EVERY_ROWS = 25;     // flush periodically for safety
