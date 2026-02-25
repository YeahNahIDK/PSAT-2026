#include "BMP390Driver.h"
#include "hardware_config.h"

BMP390Driver::BMP390Driver() {
}

bool BMP390Driver::begin() {
    if (!bmp.begin_I2C(ADDRESS_ALTIMETER)) { 
        if (!bmp.begin_I2C(ADDRESS_BACKUP_ALTIMETER)) return false;
    }

    // Settings optimized for flight
    bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_7);
    bmp.setOutputDataRate(BMP3_ODR_50_HZ);

    return true;
}

void BMP390Driver::calibrate() {
    // Read and discard 100 readings
    for (int i = 0; i < 100; i++) {
        bmp.readAltitude(SEALEVELPRESSURE_HPA);
        delay(5);
    }
    
    float total = 0;
    int num_readings = 20;

    for (int i = 0; i < num_readings; i++) {
        total += bmp.readAltitude(SEALEVELPRESSURE_HPA);
        delay(50);
    }
    
    _groundOffset = total / (double)num_readings;
}

float BMP390Driver::getAltitude() {
    float rawAlt = bmp.readAltitude(SEALEVELPRESSURE_HPA);
    return rawAlt - _groundOffset;
}

float BMP390Driver::getPressure() {
    // Pa -> hPa
    return bmp.readPressure() / 100;
}

float BMP390Driver::getTemperature() {
    return bmp.readTemperature();
}