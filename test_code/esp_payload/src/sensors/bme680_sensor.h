#pragma once
#include <Arduino.h>

struct BMEData {
  float tempC = NAN;
  float humidity = NAN;
  float pressurePa = NAN;
  float altitudeM = NAN;   // altitude computed from pressure
};

class BME680Sensor {
public:
  // Initialise the BME680 over I2C
  bool begin();

  // Read latest measurements into out
  bool read(BMEData &out);

  // Altitude helpers:
  // - Baseline altitude is the "launch pad altitude" we subtract from later
  void setAltBaseline(float baselineM);
  float getAltBaseline() const;
  float altRelative(float altitudeM) const;

  // Sea-level pressure (hPa) affects absolute altitude.
  // If you don't have a good value, 1013.25 is ok, but altitude will drift with weather.
  void setSeaLevelPressure(float hPa);
  float getSeaLevelPressure() const;

private:
  float _altBaselineM = 0.0f;
  float _seaLevelhPa = 1013.25f;
};
