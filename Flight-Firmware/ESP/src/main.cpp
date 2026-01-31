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
}

static const char *TAG = "MAIN";

LoRaHandler lora;

#define BUZZER_PIN  21
BuzzerDriver buzzer(BUZZER_PIN);

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_UART 1
GPSDriver gps(GPS_UART, GPS_RX_PIN, GPS_TX_PIN);

// Timers for non-blocking delays
unsigned long lastTelemetryTime = 0;
const long TELEMETRY_INTERVAL = 1000;

void imu_test() {
    imu_data_t imu{};
    esp_err_t e = mpu6050_read(&imu);

    if (e == ESP_OK) {
        ESP_LOGI(TAG, "A[g]=%.2f %.2f %.2f  G[dps]=%.1f %.1f %.1f",
                    imu.ax_g, imu.ay_g, imu.az_g,
                    imu.gx_dps, imu.gy_dps, imu.gz_dps);
    } else {
        ESP_LOGW(TAG, "MPU read fail: %d", (int)e);
    }
}

void lora_test() {
    if(lora.send("Ping")) {
        ESP_LOGI(TAG, "LoRa Ping Sent");
    } else {
        ESP_LOGE(TAG, "LoRa Ping Failed");
    }
}

void gps_test() {
    if (gps.isValid()) {
            ESP_LOGI(TAG, "GPS: %.6f, %.6f", gps.getLatitude(), gps.getLongitude());
    } else {
            ESP_LOGI(TAG, "GPS: No Fix (Sats: %d)", gps.getSatellites());
    }
}

void setup() {
    Serial.begin(115200);
    
    ESP_LOGI(TAG, "Booting...");

    // For c6: SCK=18, MISO=20, MOSI=19, SS=10
    SPI.begin(18, 20, 19, 10);

    ESP_ERROR_CHECK(my_i2c_init());
    ESP_ERROR_CHECK(mpu6050_init());
    if (!lora.init()) {
        ESP_LOGE(TAG, "LoRa Init Failed!");
    }
    if (!gps.begin()) {
        ESP_LOGE(TAG, "GPS Init Failed!");
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
        imu_test();
        lora_test();
        gps_test();
    }
}
