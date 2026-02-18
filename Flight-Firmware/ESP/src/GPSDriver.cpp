#include "GPSDriver.h"

static const char *TAG = "GPS";

// Constructor implementation
// Initialize the HardwareSerial with the UART number passed (0, 1, or 2)
GPSDriver::GPSDriver(int uartNr, int rxPin, int txPin, long baudRate) 
    : _serial(uartNr), _rxPin(rxPin), _txPin(txPin), _baudRate(baudRate) {
}

bool GPSDriver::begin() {
    // Initialize the hardware serial port
    _serial.setRxBufferSize(1024);
    
    // SERIAL_8N1 is the standard configuration for NMEA
    _serial.begin(_baudRate, SERIAL_8N1, _rxPin, _txPin);

    ESP_LOGI(TAG, "Initializing connection to PA1616D...");

    // Test connection
    unsigned long start = millis();
    bool trafficDetected = false;

    while (millis() - start < 1500) {
        if (_serial.available() > 0) {
            trafficDetected = true;
            break; // We found the GPS! Exit loop early.
        }
    }

    if (trafficDetected) {
        ESP_LOGI(TAG, "SUCCESS! Data stream detected.");
        return true;
    } else {
        ESP_LOGE(TAG, "FAILED! No data received.");
        return false;
    }
}

bool GPSDriver::update() {
    bool newData = false;
    // Read all available bytes from the serial buffer
    while (_serial.available() > 0) {
        char c = _serial.read();
        // Feed the parser
        if (tGps.encode(c)) {
            newData = true;
        }
    }
    return newData;
}

double GPSDriver::getLatitude() {
    return tGps.location.lat();
}

double GPSDriver::getLongitude() {
    return tGps.location.lng();
}

double GPSDriver::getAltitude() {
    return tGps.altitude.meters();
}

int GPSDriver::getSatellites() {
    return tGps.satellites.value();
}

bool GPSDriver::isValid() {
    return tGps.location.isValid();
}