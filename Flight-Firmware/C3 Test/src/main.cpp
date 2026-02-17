#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

#include "BMP390Driver.h"
#include "ICMDriver.h"

static const char *TAG = "MAIN";

BMP390Driver altimeter;
ICMDriver imu(Wire, 0x69);

// Timers for non-blocking delays
unsigned long lastTelemetryTime = 0;
const long TELEMETRY_INTERVAL = 1000;

void imu_test() {
    if (imu.update()) {
        IMUData d = imu.getData();
        Serial.printf("A: %.2f %.2f %.2f | G: %.2f %.2f %.2f\n", 
            d.accX, d.accY, d.accZ, 
            d.gyrX, d.gyrY, d.gyrZ);
    }
}

void setup() {
    delay(2000);
    Serial.begin(115200);

    // SDA, SCL
    Wire.begin(20, 21);

    if (!altimeter.begin()) {
        ESP_LOGE(TAG, "BMP390 Init Failed!");
    }

    if (!imu.begin()) {
         ESP_LOGE(TAG, "IMU Init Failed!");
    }
    
    delay(500);

    altimeter.calibrate();
}

void loop() {
    Serial.println(altimeter.getAltitude());
    imu_test();
    delay(500);
}
