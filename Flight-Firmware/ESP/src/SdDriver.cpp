#include "SdDriver.h"

SdDriver::SdDriver(int csPin) : _csPin(csPin), _isInitialized(false) {}

bool SdDriver::begin(SPIClass &spiBus) {
    if (!SD.begin(_csPin, spiBus)) {
        Serial.println("SD Mount Failed");
        return false;
    }
    _isInitialized = true;
    return true;
}

bool SdDriver::openLog(const char* path) {
    if (!_isInitialized) return false;
    
    // Open the file once in append mode and leave it open
    _logFile = SD.open(path, FILE_APPEND);
    return _logFile == true;
}

void SdDriver::logData(const char* data) {
    if (_logFile) {
        // Writes to the internal RAM buffer. 
        // When it hits 512 bytes, it automatically physically saves to the SD card.
        _logFile.print(data); 
    }
}

void SdDriver::save() {
    if (_logFile) {
        // Manually forces a physical write, regardless of buffer size
        _logFile.flush(); 
    }
}

void SdDriver::closeLog() {
    if (_logFile) {
        _logFile.close();
    }
}