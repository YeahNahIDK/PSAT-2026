#include "ICMDriver.h"

// --- Registers for ICM-42670-P ---
#define REG_MCLK_RDY        0x00 
#define REG_ACCEL_DATA_X1   0x0B
#define REG_PWR_MGMT0       0x1F
#define REG_GYRO_CONFIG0    0x20
#define REG_ACCEL_CONFIG0   0x21
#define REG_WHO_AM_I        0x75
#define DEVICE_ID           0x67

ICMDriver::ICMDriver(TwoWire &wirePort, uint8_t addr) {
    _wire = &wirePort;
    _addr = addr;
}

bool ICMDriver::begin() {
    uint8_t whoAmI = readRegister(REG_WHO_AM_I);
    if (whoAmI != DEVICE_ID ) {
        return false;
    }

    /* Power Management (PWR_MGMT0) */
    // Bit 3:2 = Gyro Mode (11 = Low Noise)
    // Bit 1:0 = Accel Mode (11 = Low Noise)
    // Writing 0x0F turns everything ON in High Precision mode
    writeRegister(REG_PWR_MGMT0, 0x0F);
    delay(50); // Wait for sensors to spin up

    /* Configure Gyro (GYRO_CONFIG0) */
    // Bits 7:5 = Range (000 = +/- 2000 dps)
    // Bits 3:0 = ODR (0111 = 100Hz)
    // 0x00 | 0x07 = 0x07
    writeRegister(REG_GYRO_CONFIG0, 0x07);

    /* Configure Accel (ACCEL_CONFIG0) */
    // Bits 7:5 = Range (000 = +/- 16g)
    // Bits 3:0 = ODR (0111 = 100Hz)
    // 0x00 | 0x07 = 0x07
    writeRegister(REG_ACCEL_CONFIG0, 0x07);

    return true;
}

bool ICMDriver::update() {
    _wire->beginTransmission(_addr);
    _wire->write(REG_ACCEL_DATA_X1); // Start reading from Accel X High Byte
    if (_wire->endTransmission(false) != 0) return false;

    // Request 12 bytes: Accel X/Y/Z (6) + Gyro X/Y/Z (6)
    if (_wire->requestFrom(_addr, (uint8_t)12) != 12) return false;

    int16_t rawAx = (_wire->read() << 8) | _wire->read();
    int16_t rawAy = (_wire->read() << 8) | _wire->read();
    int16_t rawAz = (_wire->read() << 8) | _wire->read();
    int16_t rawGx = (_wire->read() << 8) | _wire->read();
    int16_t rawGy = (_wire->read() << 8) | _wire->read();
    int16_t rawGz = (_wire->read() << 8) | _wire->read();

    // Convert to physical units
    // Accel 16g scale: Sensitivity is 2048 LSB/g
    _data.accX = rawAx / 2048.0f;
    _data.accY = rawAy / 2048.0f;
    _data.accZ = rawAz / 2048.0f;

    // Gyro 2000dps scale: Sensitivity is 16.4 LSB/dps
    _data.gyrX = (rawGx / 16.4f) - _gyroOffsetX;
    _data.gyrY = (rawGy / 16.4f) - _gyroOffsetY;
    _data.gyrZ = (rawGz / 16.4f) - _gyroOffsetZ;

    return true;
}

IMUData ICMDriver::getData() const {
    return _data;
}

void ICMDriver::writeRegister(uint8_t reg, uint8_t data) {
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    _wire->write(data);
    _wire->endTransmission();
}

uint8_t ICMDriver::readRegister(uint8_t reg) {
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    _wire->endTransmission(false);
    _wire->requestFrom(_addr, (uint8_t)1);
    return _wire->read();
}

void ICMDriver::calibrateGyro() {
    float totalX = 0, totalY = 0, totalZ = 0;
    int num_readings = 200;

    // 1. Reset offsets to 0 so they don't skew the raw readings
    _gyroOffsetX = 0.0f;
    _gyroOffsetY = 0.0f;
    _gyroOffsetZ = 0.0f;

    // 2. Discard the first few readings (flushes stale data)
    for (int i = 0; i < 50; i++) {
        update();
        delay(10); // Wait 10ms for the next 100Hz reading
    }

    // 3. Accumulate readings
    for (int i = 0; i < num_readings; i++) {
        update();
        totalX += _data.gyrX;
        totalY += _data.gyrY;
        totalZ += _data.gyrZ;
        delay(10); 
    }

    // 4. Calculate and store the averages
    _gyroOffsetX = totalX / (float)num_readings;
    _gyroOffsetY = totalY / (float)num_readings;
    _gyroOffsetZ = totalZ / (float)num_readings;
}
