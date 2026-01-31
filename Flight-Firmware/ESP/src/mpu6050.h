#pragma once
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float ax_g, ay_g, az_g;
    float gx_dps, gy_dps, gz_dps;
} imu_data_t;

esp_err_t mpu6050_init(void);
esp_err_t mpu6050_read(imu_data_t *out);

#ifdef __cplusplus
}
#endif
