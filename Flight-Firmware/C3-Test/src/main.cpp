#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

#include "BMP390Driver.h"
#include "ICMDriver.h"
#include "SdDriver.h"

#define PIN_SPI_MOSI                10
#define PIN_SPI_MISO                5
#define PIN_SPI_SCK                 20
#define PIN_SD_CS                   3 

static const char *TAG = "MAIN";

BMP390Driver altimeter;
ICMDriver imu(Wire, 0x69);
SdDriver sd(PIN_SD_CS);

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
    Serial.setDebugOutput(true);
    
    delay(2000);

    // SDA, SCL
    Wire.begin(0, 1);
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, -1);

    // if (!altimeter.begin()) {
    //     ESP_LOGE(TAG, "BMP390 Init Failed!");
    // }

    // if (!imu.begin()) {
    //      ESP_LOGE(TAG, "IMU Init Failed!");
    // }

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
    
    delay(500);

    // altimeter.calibrate();
}

void loop() {
    // Serial.println(altimeter.getAltitude());
    // imu_test();
    delay(500);
}
