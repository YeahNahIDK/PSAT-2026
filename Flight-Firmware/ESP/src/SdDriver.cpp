#include "SdDriver.h"

SdDriver::SdDriver(int csPin) : _csPin(csPin), _isInitialized(false) {}

bool SdDriver::begin(SPIClass &spiBus) {
    SdSpiConfig spiConfig(_csPin, SHARED_SPI, SD_SCK_MHZ(4), &spiBus);

    if (!_sd.begin(spiConfig)) {
        return false;
    }
    _isInitialized = true;
    return true;
}

bool SdDriver::openLog(const char* path) {
    if (!_isInitialized) return false;
    
    // Creates the file if it doesn't exist, opens for writing, and moves to the end.
    if (!_logFile.open(path, O_WRITE | O_CREAT | O_AT_END)) {
        return false;
    }
    return true;
}

void SdDriver::logData(const char* data) {
    if (_logFile) {
        _logFile.print(data); 
    }
}

void SdDriver::save() {
    if (_logFile) {
        // Forces the buffer to physically write to the SD card.
        _logFile.sync(); 
    }
}

void SdDriver::closeLog() {
    if (_logFile) {
        _logFile.close();
    }
}

void SdDriver::getErrorDetails(char* buffer) {
    // sdErrorCode() and sdErrorData() are built into SdFat
    uint8_t code = _sd.sdErrorCode();
    uint8_t data = _sd.sdErrorData();

    if (code == 0) {
        // If the hardware code is 0 but it still failed, it's a file system issue.
        sprintf(buffer, "SD ERR: File System (Not FAT32/exFAT)");
    } else {
        // Otherwise, it's a hardware/SPI/wiring issue. We format it as Hex.
        sprintf(buffer, "SD ERR: HW Code 0x%X, Data 0x%X", code, data);
    }
}
