#include "mpu6050.h"
#include "my_i2c.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "MPU6050";

#define MPU6050_ADDR        0x68

#define REG_WHO_AM_I        0x75
#define REG_PWR_MGMT_1      0x6B
#define REG_ACCEL_XOUT_H    0x3B

static int16_t be16(const uint8_t *p) { return (int16_t)((p[0] << 8) | p[1]); }

esp_err_t mpu6050_init(void)
{
    uint8_t who = 0;

    // Wake it up
    esp_err_t err = my_i2c_write_reg(MPU6050_ADDR, REG_PWR_MGMT_1, 0x00);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "PWR_MGMT_1 write failed: %d", err);
        return err;
    }

    vTaskDelay(pdMS_TO_TICKS(50));

    err = my_i2c_read_regs(MPU6050_ADDR, REG_WHO_AM_I, &who, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "WHO_AM_I read failed: %d", err);
        return err;
    }

    ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", who);
    return (who == 0x68) ? ESP_OK : ESP_FAIL;
}

esp_err_t mpu6050_read(imu_data_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;

    uint8_t buf[14] = {0};
    esp_err_t err = my_i2c_read_regs(MPU6050_ADDR, REG_ACCEL_XOUT_H, buf, sizeof(buf));
    if (err != ESP_OK) return err;

    int16_t ax = be16(&buf[0]);
    int16_t ay = be16(&buf[2]);
    int16_t az = be16(&buf[4]);
    int16_t gx = be16(&buf[8]);
    int16_t gy = be16(&buf[10]);
    int16_t gz = be16(&buf[12]);

    // Defaults: accel ±2g => 16384 LSB/g, gyro ±250 dps => 131 LSB/(dps)
    out->ax_g = (float)ax / 16384.0f;
    out->ay_g = (float)ay / 16384.0f;
    out->az_g = (float)az / 16384.0f;

    out->gx_dps = (float)gx / 131.0f;
    out->gy_dps = (float)gy / 131.0f;
    out->gz_dps = (float)gz / 131.0f;

    return ESP_OK;
}
