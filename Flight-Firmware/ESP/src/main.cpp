#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

#include "LoRaHandler.h"

extern "C" {
#include "my_i2c.h"
#include "mpu6050.h"
}

static const char *TAG = "MAIN";

LoRaHandler lora;

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


void setup() {
    ESP_LOGI(TAG, "Booting...");

    // For c6: SCK=18, MISO=20, MOSI=19, SS=10
    SPI.begin(18, 20, 19, 10);

    ESP_ERROR_CHECK(my_i2c_init());
    ESP_ERROR_CHECK(mpu6050_init());
    if (!lora.init()) {
        ESP_LOGE(TAG, "LoRa Init Failed!");
    }
    delay(1000);
}

void loop() {
    imu_test();
    lora_test();

    delay(1000);
}
