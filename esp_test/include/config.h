#pragma once

// this needs to be updated - ayesha


// CONFIGURATION FILE
// This file holds pin assignments and system-wide constants
// Instead of repeating pin numbers in every file, we define them once here

// I2C Pins — used for both IMU and Altimeter (shared bus)
#define PIN_SDA       21   // Data line
#define PIN_SCL       22   // Clock line

// Optional interrupt pin for IMU (you can ignore if not using it)
#define PIN_IMU_INT   4

// SPI Pins — used for SD card module
#define PIN_SD_MOSI   23
#define PIN_SD_MISO   19
#define PIN_SD_SCLK   18
#define PIN_SD_CS     13   // Chip Select — tells ESP32 when to talk to SD

// Peripherals
#define PIN_BUZZER    26   // Connect buzzer signal wire here
#define PIN_LED       2    // Use onboard LED or external one on this pin

// System timing
#define LOG_INTERVAL  100  // How often to log sensor data (milliseconds)

// Igniter Main 
#define PIN_IGNITER       27      // TODO: Update to actual pin
#define ALT_MAIN_DEPLOY   250.0   // Altitude in meters for main parachute

// UART Communication
#define UART_TX_TO_IGNITER 17  // Replace with our TX pin
#define UART_RX_FROM_IGNITER 16  // Optional unless we want to receive back ¯\_(ツ)_/¯


// Uncomment this during bench testing to disable actual igniter firing
// #define TEST_MODE