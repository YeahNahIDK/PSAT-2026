#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "altimeter.h"

#define DEFAULT_I2C_SDA 21
#define DEFAULT_I2C_SCL 22
#define DEFAULT_I2C_ADDR 0x76
#define SEA_LEVEL_HPA    1013.25f

// BMP280 sampling (~16 Hz effective ODR)
#define BMP_TEMP_OVERSAMPL  Adafruit_BMP280::SAMPLING_X2
#define BMP_PRESS_OVERSAMPL Adafruit_BMP280::SAMPLING_X16
#define BMP_FILTER          Adafruit_BMP280::FILTER_X16
#define BMP_STANDBY         Adafruit_BMP280::STANDBY_MS_63

// --- Module state ---
static Adafruit_BMP280 bmp;
static uint8_t sdaPin  = DEFAULT_I2C_SDA;
static uint8_t sclPin  = DEFAULT_I2C_SCL;
static uint8_t i2cAddr = DEFAULT_I2C_ADDR;

static float g_p0_hPa     = SEA_LEVEL_HPA;
static bool  g_haveP0     = false;
static bool  g_zeroed     = false;
static float g_groundAltM = 0.0f;

// ---- Calibration state (non-blocking) ----
struct PressureCalibrator {
  int targetSamples;
  int collected;
  unsigned long lastSampleTime;
  int delayBetween;
  float sum_hPa;
  bool finished;

  PressureCalibrator() :
    targetSamples(0), collected(0),
    lastSampleTime(0), delayBetween(0),
    sum_hPa(0.0f), finished(false) {}
};

static PressureCalibrator g_cal;

// ---- Internals ----
static bool readAbsolute(float &p_hPa, float &alt_m) {
  float p_Pa = bmp.readPressure();   // Pa
  float alt  = bmp.readAltitude(g_haveP0 ? g_p0_hPa : SEA_LEVEL_HPA);

  if (isnan(p_Pa) || isnan(alt)) return false;

  p_hPa = p_Pa * 0.01f; // Pa → hPa
  alt_m = alt;
  return true;
}

// ---- Public API ----
void configureAltimeterPins(uint8_t sda, uint8_t scl) {
  sdaPin = sda;
  sclPin = scl;
}

void configureAltimeterAddress(uint8_t address) {
  i2cAddr = address;
}

void initAltimeter() {
  Wire.begin(sdaPin, sclPin);

  if (!bmp.begin(i2cAddr)) {
    Serial.println("[ALT] ERROR: BMP280 not found. Check wiring/address.");
    return;
  }

  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    BMP_TEMP_OVERSAMPL,
    BMP_PRESS_OVERSAMPL,
    BMP_FILTER,
    BMP_STANDBY
  );

  Serial.println("[ALT] BMP280 initialised.");

  // Option 1: blocking calibration
  // g_p0_hPa = calibrateGroundPressure(20, 50);

  // Option 2 (recommended): start non-blocking calibration
  startAltimeterCalibration(20, 50);
}

bool readAltimeter(float &pressure_hPa, float &altitude_m) {
  float p_hPa, a_m;
  if (!readAbsolute(p_hPa, a_m)) return false;
  pressure_hPa = p_hPa;
  altitude_m   = a_m;
  return true;
}

bool readAltimeterRelative(float &pressure_hPa, float &relAltitude_m) {
  float p_hPa, a_m;
  if (!readAbsolute(p_hPa, a_m)) return false;

  if (!g_zeroed) zeroAltimeter();

  pressure_hPa  = p_hPa;
  relAltitude_m = a_m - g_groundAltM;
  return true;
}

void setSeaLevelPressure(float hPa) {
  g_p0_hPa = hPa;
  g_haveP0 = true;
  Serial.printf("[ALT] p0 overridden: %.2f hPa\n", g_p0_hPa);

  g_zeroed = false;
  zeroAltimeter();
}

void zeroAltimeter() {
  float p_hPa, a_m;
  if (readAbsolute(p_hPa, a_m)) {
    g_groundAltM = a_m;
    g_zeroed     = true;
    Serial.printf("[ALT] Zeroed at %.2f m (relative = 0.00 m)\n", g_groundAltM);
  }
}

float estimateAltitude(float pressure_hPa, float seaLevelPressure_hPa) {
  return 44330.0f * (1.0f - powf(pressure_hPa / seaLevelPressure_hPa, 0.1903f));
}

// --- Legacy blocking calibration ---
float calibrateGroundPressure(int samples, int delayBetweenMs) {
  float sum_hPa = 0.0f;
  int valid = 0;

  for (int i = 0; i < samples; i++) {
    float p_Pa = bmp.readPressure();
    if (!isnan(p_Pa)) {
      sum_hPa += p_Pa * 0.01f;
      valid++;
    }
    delay(delayBetweenMs); // blocking
  }

  if (valid == 0) {
    Serial.println("[ALT] WARN: calibration failed, using ISA p0.");
    return SEA_LEVEL_HPA;
  }
  return sum_hPa / valid;
}

// --- Non-blocking calibration API ---
void startAltimeterCalibration(int samples, int delayMs) {
  g_cal.targetSamples  = samples;
  g_cal.delayBetween   = delayMs;
  g_cal.collected      = 0;
  g_cal.sum_hPa        = 0.0f;
  g_cal.lastSampleTime = millis();
  g_cal.finished       = false;
}

bool updateAltimeterCalibration(float &out_hPa, bool &done) {
  done = false;

  if (g_cal.finished) {
    out_hPa = (g_cal.collected > 0)
              ? g_cal.sum_hPa / g_cal.collected
              : SEA_LEVEL_HPA;
    done = true;
    return true;
  }

  unsigned long now = millis();
  if (now - g_cal.lastSampleTime >= (unsigned long)g_cal.delayBetween &&
      g_cal.collected < g_cal.targetSamples) {

    g_cal.lastSampleTime = now;
    float p_Pa = bmp.readPressure();
    if (!isnan(p_Pa)) {
      g_cal.sum_hPa += p_Pa * 0.01f;
      g_cal.collected++;
    }
  }

  if (g_cal.collected >= g_cal.targetSamples) {
    g_cal.finished = true;
    out_hPa = g_cal.sum_hPa / g_cal.collected;
    done = true;
    return true;
  }

  return false; // still collecting
}

// --- Convenience accessors ---
float readPressure_hPa() {
  float p_Pa = bmp.readPressure();
  if (isnan(p_Pa)) return NAN;
  return p_Pa * 0.01f;
}

float readTemperature_C() {
  float t = bmp.readTemperature();
  if (isnan(t)) return NAN;
  return t;
}




