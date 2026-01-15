#pragma once
#include <Arduino.h>

// ===== I2C for BME680 + MPU6050 =====
static const int PIN_I2C_SDA = 8;
static const int PIN_I2C_SCL = 9;

// ===== SD SPI =====
static const int PIN_SD_SCK  = 4;
static const int PIN_SD_MISO = 5;
static const int PIN_SD_MOSI = 6;
static const int PIN_SD_CS   = 7;

// ===== UART to MSP430 LoRa controller (use RX/TX labeled pins) =====
static const int PIN_MSP_LORA_RX = 20; // ESP32 RX  <- MSP TX (optional)
static const int PIN_MSP_LORA_TX = 21; // ESP32 TX  -> MSP RX

// ===== UART to Arduino servo test =====
static const int PIN_ARDUINO_RX = 3;  // ESP32 RX <- Arduino TX (optional)
static const int PIN_ARDUINO_TX = 2;  // ESP32 TX -> Arduino RX (SoftwareSerial RX)

static const uint32_t BAUD_DEBUG = 115200;
static const uint32_t BAUD_LINK  = 115200;
