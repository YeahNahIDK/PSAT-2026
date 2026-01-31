#pragma once
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float temp_C;
    float press_hPa;
    float hum_pct;
    float altitude_m;
} bme_data_t;

// init sensor + capture baseline pressure (for altitude)
esp_err_t bme680_init(void);

// read temp/press/humidity + altitude (using baseline captured in init)
esp_err_t bme680_read(bme_data_t *out);

// optional: call again if you want to re-zero altitude on the pad
void bme680_recalibrate_baseline(void);

#ifdef __cplusplus
}
#endif
