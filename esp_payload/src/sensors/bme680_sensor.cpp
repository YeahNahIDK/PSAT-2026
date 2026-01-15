#include "bme680_sensor.h"
#include <Adafruit_BME680.h>

// One global sensor instance (library object)
static Adafruit_BME680 bme;

bool BME680Sensor::begin() {
  // begin() uses the already-initialised Wire bus
  if (!bme.begin()) return false;

  // Oversampling + filter:
  // More stable pressure/altitude without being too slow.
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  // Gas sensor is not needed for altitude/temp/humidity logging
  bme.setGasHeater(0, 0);

  return true;
}

bool BME680Sensor::read(BMEData &out) {
  if (!bme.performReading()) return false;

  out.tempC = bme.temperature;
  out.humidity = bme.humidity;
  out.pressurePa = bme.pressure;  // Pa

  // readAltitude expects sea-level pressure in hPa
  out.altitudeM = bme.readAltitude(_seaLevelhPa);
  return true;
}

void BME680Sensor::setAltBaseline(float baselineM) { _altBaselineM = baselineM; }
float BME680Sensor::getAltBaseline() const { return _altBaselineM; }

float BME680Sensor::altRelative(float altitudeM) const {
  return altitudeM - _altBaselineM;
}

void BME680Sensor::setSeaLevelPressure(float hPa) { _seaLevelhPa = hPa; }
float BME680Sensor::getSeaLevelPressure() const { return _seaLevelhPa; }
