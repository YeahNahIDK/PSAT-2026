#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
#include "my_i2c.h"
#include "mpu6050.h"
}

static const char *TAG = "MAIN";

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

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Boot");

    ESP_ERROR_CHECK(my_i2c_init());
    ESP_ERROR_CHECK(mpu6050_init());

    imu_test();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(50)); // 20 Hz
    }
}
