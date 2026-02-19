#pragma once
#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SdDriver {
private:
    int _csPin;
    bool _isInitialized;
    File _logFile; // Keeps the file open continuously

public:
    SdDriver(int csPin);
    bool begin(SPIClass &spiBus);
    
    // Core logging methods
    bool openLog(const char* path);
    void logData(const char* data);
    
    // Manual overrides
    void save();   
    void closeLog(); 
};