#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>
#include <SPI.h>
#include "SdDriver.h"

#define CS_PIN -1

static const char *TAG = "MAIN";

SdDriver sd(20);

// Timers for non-blocking delays
unsigned long lastTelemetryTime = 0;
const long TELEMETRY_INTERVAL = 1000;

void setup() {
    Serial.begin(115200);
    // SCK=21, MISO=23, MOSI=22, CS=20
    SPI.begin(21, 23, 22, -1);

    bool sd_success = sd.begin(SPI);
    if (sd_success) {
        Serial.println("Success");
    } else {
        Serial.println("Failure");
    }
    delay(250);
    if (!sd_success) {
        char sd_error_msg[64] = {0};
        sd.getErrorDetails(sd_error_msg);
        
        Serial.println(sd_error_msg);
    } 
    else {
        bool write_sucess = sd.openLog("/flight_data.csv");
        if (write_sucess) {
            Serial.println("Sucess");
            sd.logData("Time (ms), Altitude (m), Temp (°C), "
           "Accel_X (G), Accel_Y (G), Accel_Z (G), "
           "Gyro_X (dps), Gyro_Y (dps), Gyro_Z (dps), "
           "Latitude, Longitude\n");
            sd.save(); 
        } else { Serial.println("Failure"); }
    }

}

void loop() {
    delay(500);
}
