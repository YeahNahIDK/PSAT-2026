#pragma once
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif


#define I2C_SDA_PIN  (8)
#define I2C_SCL_PIN  (9)

esp_err_t my_i2c_init(void);

esp_err_t my_i2c_read_reg(uint8_t dev, uint8_t reg, uint8_t *out);
esp_err_t my_i2c_write_reg(uint8_t dev, uint8_t reg, uint8_t val);
esp_err_t my_i2c_read_regs(uint8_t dev, uint8_t start_reg, uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif
