#include "my_i2c.h"

#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MY_I2C";

// ===== YOU MUST SET THESE to your PCB pins =====
#define I2C_PORT        I2C_NUM_0
#define I2C_SDA_GPIO    6     // <-- CHANGE ME
#define I2C_SCL_GPIO    7     // <-- CHANGE ME
#define I2C_FREQ_HZ     100000

static bool s_installed = false;

esp_err_t my_i2c_init(void)
{
    if (s_installed) return ESP_OK;

    i2c_config_t cfg = {0};
    cfg.mode = I2C_MODE_MASTER;
    cfg.sda_io_num = I2C_SDA_GPIO;
    cfg.scl_io_num = I2C_SCL_GPIO;
    cfg.sda_pullup_en = GPIO_PULLUP_ENABLE;
    cfg.scl_pullup_en = GPIO_PULLUP_ENABLE;
    cfg.master.clk_speed = I2C_FREQ_HZ;

    esp_err_t err = i2c_param_config(I2C_PORT, &cfg);
    if (err != ESP_OK) return err;

    err = i2c_driver_install(I2C_PORT, cfg.mode, 0, 0, 0);
    if (err != ESP_OK) return err;

    s_installed = true;
    ESP_LOGI(TAG, "I2C init OK (SDA=%d SCL=%d @%dHz)", I2C_SDA_GPIO, I2C_SCL_GPIO, I2C_FREQ_HZ);
    return ESP_OK;
}

esp_err_t my_i2c_write_reg(uint8_t dev, uint8_t reg, uint8_t val)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, val, true);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(200));
    i2c_cmd_link_delete(cmd);
    return err;
}

esp_err_t my_i2c_read_regs(uint8_t dev, uint8_t start_reg, uint8_t *buf, uint16_t len)
{
    if (!buf || len == 0) return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Write register address
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, start_reg, true);

    // Re-start and read
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_READ, true);

    if (len > 1)
        i2c_master_read(cmd, buf, len - 1, I2C_MASTER_ACK);

    i2c_master_read_byte(cmd, buf + (len - 1), I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(200));
    i2c_cmd_link_delete(cmd);
    return err;
}
