
// baro.c — IMPLEMENTATION (PSEUDOCODE)

// BMP388 uses I2C registers. We periodically read raw pressure and
// temperature, then convert using formulas from the datasheet.


static float pressure_pa;
static float temperature_c;
static float altitude_m;

void baro_init() {
    // I2C mode enabled
    // Read calibration coefficients
    // Configure oversampling and filter (from Adafruit example)
}

void baro_update() {
    // Read raw temperature + pressure registers
    // Apply calibration compensation
    // Convert to altitude using standard barometric formula
}

float baro_pressure()    { return pressure_pa; }
float baro_temperature() { return temperature_c; }
float baro_altitude()    { return altitude_m; }
