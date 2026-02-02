// Barometer (BMP280)
#pragma once
#include <Arduino.h>

// ---- Init ----

// Initialises I2C + BMP280, kicks off calibration, and zeroes altitude.
// Safe to call once in setup(). Prints status to Serial.
void initAltimeter();

// ---- Reads ----

// Absolute read using current p0 (hPa and meters above sea level).
// Returns true if a fresh, valid reading was obtained.
bool readAltimeter(float &pressure_hPa, float &altitude_m);

// Relative read: altitude is zeroed at the last zeroAltimeter() call.
// Pressure is still absolute (hPa). Returns true on success.
bool readAltimeterRelative(float &pressure_hPa, float &relAltitude_m);

// ---- Calibration ----

// Start a non-blocking ground pressure calibration (average of N samples).
void startAltimeterCalibration(int samples, int delayMs);

// Poll the calibration process. Returns true once finished.
// `out_hPa` = averaged sea-level pressure estimate.
// `done` flag tells you if calibration is complete.
bool updateAltimeterCalibration(float &out_hPa, bool &done);

// Blocking version (legacy). Avoid in flight loops.
float calibrateGroundPressure(int samples, int delayBetweenMs);

// ---- Configuration ----

// Manually set sea-level pressure p0 (in hPa), e.g., from a weather station.
// Re-zeros the relative baseline automatically.
void setSeaLevelPressure(float hPa);

// Zero the relative altitude to “here and now” (sets baseline for relative reads).
void zeroAltimeter();

// ---- Helpers ----

// Compute altitude (m) from pressure(hPa) and sea-level p0(hPa).
float estimateAltitude(float pressure_hPa, float seaLevelPressure_hPa);

// Optional convenience accessors (return NAN on failure).
float readPressure_hPa();
float readTemperature_C();

// If we need to change I2C pins or address at runtime.
void configureAltimeterPins(uint8_t sda, uint8_t scl);
void configureAltimeterAddress(uint8_t i2cAddress);
