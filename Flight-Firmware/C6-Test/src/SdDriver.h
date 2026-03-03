#ifndef SD_DRIVER_H
#define SD_DRIVER_H

#include <SPI.h>
#include "SdFat.h"

class SdDriver {
public:
    SdDriver(int csPin);
    bool begin(SPIClass &spiBus);
    bool openLog(const char* path);
    void logData(const char* data);
    void save();
    void closeLog();

    void getErrorDetails(char* buffer);

private:
    int _csPin;
    bool _isInitialized;
    
    // Universal file system object
    SdFs _sd;          
    
    // Universal file object
    FsFile _logFile;   
};

#endif