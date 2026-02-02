#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

#include "LoRaHandler.h"
#include "GPSDriver.h"
#include "BuzzerDriver.h"

extern "C" {
    #include "my_i2c.h"
    #include "mpu6050.h"
    #include "bme680_app.h"
}

static const char *TAG = "MAIN";

LoRaHandler lora;

#define BUZZER_PIN  21
BuzzerDriver buzzer(BUZZER_PIN);

#define GPS_RX_PIN 5 // Wire to ESP TX
#define GPS_TX_PIN 4
#define GPS_UART 1
GPSDriver gps(GPS_UART, GPS_RX_PIN, GPS_TX_PIN);

// Timers for non-blocking delays
unsigned long lastTelemetryTime = 0;
const long TELEMETRY_INTERVAL = 1000;

void imu_test() {
    imu_data_t imu{};
    esp_err_t e = mpu6050_read(&imu);

    if (e == ESP_OK) {
        ESP_LOGI(TAG, "A[g]=%.2f %.2f %.2f  G[dps]=%.1f %.1f %.1f\n",
                    imu.ax_g, imu.ay_g, imu.az_g,
                    imu.gx_dps, imu.gy_dps, imu.gz_dps);
    } else {
        ESP_LOGW(TAG, "MPU read fail: %d\n", (int)e);
    }
}

void lora_test() {
    if(lora.send("Ping")) {
        ESP_LOGI(TAG, "LoRa Ping Sent\n");
    } else {
        ESP_LOGE(TAG, "LoRa Ping Failed\n");
    }
}

void gps_test() {
    if (gps.isValid()) {
            ESP_LOGI(TAG, "GPS: %.6f, %.6f\n", gps.getLatitude(), gps.getLongitude());
    } else {
            ESP_LOGI(TAG, "GPS: No Fix (Sats: %d)\n", gps.getSatellites());
    }
}

void baro_test() {
    bme_data_t bme_data;
    float currentAlt = 0.0;

    if (bme680_read(&bme_data) == ESP_OK) {
            currentAlt = bme_data.altitude_m;
            ESP_LOGI(TAG, "Alt: %.2fm  Press: %.2fhPa", bme_data.altitude_m, bme_data.press_hPa);
    } else {
            ESP_LOGE(TAG, "BME Read Failed");
    }
}

void setup() {
    Serial.begin(115200);
    
    Serial.println("Booting...");

    // For c6: SCK=18, MISO=20, MOSI=19, SS=9
    SPI.begin(18, 20, 19, 9);

    esp_err_t i2c_err = my_i2c_init();
    if (i2c_err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Init Failed: %s", esp_err_to_name(i2c_err));
    }

    esp_err_t mpu_err = mpu6050_init();
    if (mpu_err != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 Init Failed: %s", esp_err_to_name(mpu_err));
    } 

    if (!lora.init()) {
        ESP_LOGE(TAG, "LoRa Init Failed!\n");
    }

    if (!gps.begin()) {
        ESP_LOGE(TAG, "GPS Init Failed!\n");
    }

    buzzer.begin();
    
    delay(1000);
}

void loop() {
    gps.update();
    buzzer.update();

    if (millis() - lastTelemetryTime > TELEMETRY_INTERVAL) {
        lastTelemetryTime = millis();
        
        buzzer.beep(1, 50);

        baro_test();
        imu_test();
        lora_test();
        gps_test();
    }
}
