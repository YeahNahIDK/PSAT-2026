// src/bme680_app.c
#include "bme680_app.h"

#include <math.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"          // esp_rom_delay_us
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Your custom I2C wrapper (must exist)
#include "my_i2c.h"

// Bosch BME68x SensorAPI header
// Adjust include path to match where you placed the Bosch library.
#include "bme68x.h"

static const char *TAG = "BME680";

// -----------------------------------------------------------------------------
// Config
// -----------------------------------------------------------------------------
#define BME680_I2C_ADDR         (0x77)   // your scan showed 0x77
#define SEA_LEVEL_HPA_DEFAULT   (1013.25f)

// If you want to “zero” altitude to your current room altitude at boot,
// we keep a baseline offset.
static float g_sea_level_hpa = SEA_LEVEL_HPA_DEFAULT;
static float g_altitude_offset_m = 0.0f;
static bool  g_ready = false;

// Bosch device struct (kept static)
static struct bme68x_dev g_dev;

// -----------------------------------------------------------------------------
// Bosch callback glue (uses your my_i2c_* functions)
// Bosch passes intf_ptr — we store the 7-bit address there.
// -----------------------------------------------------------------------------
static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
{
    if (!data || !intf_ptr) return BME68X_E_NULL_PTR;

    uint8_t addr = *(uint8_t *)intf_ptr;
    esp_err_t err = my_i2c_read_regs(addr, reg_addr, data, (uint16_t)len);

    return (err == ESP_OK) ? BME68X_OK : BME68X_E_COM_FAIL;
}

static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
{
    if (!data || !intf_ptr) return BME68X_E_NULL_PTR;

    uint8_t addr = *(uint8_t *)intf_ptr;

    // We only have a single-byte register write helper, so write sequentially.
    // (Works fine for this sensor at low speed)
    for (uint32_t i = 0; i < len; i++)
    {
        esp_err_t err = my_i2c_write_reg(addr, (uint8_t)(reg_addr + i), data[i]);
        if (err != ESP_OK) return BME68X_E_COM_FAIL;
    }
    return BME68X_OK;
}

static void user_delay_us(uint32_t period_us, void *intf_ptr)
{
    (void)intf_ptr;

    // For short delays use ROM delay
    if (period_us < 20000)
    {
        esp_rom_delay_us(period_us);
        return;
    }

    // For longer delays, yield to FreeRTOS
    uint32_t ms = (period_us + 999) / 1000;
    vTaskDelay(pdMS_TO_TICKS(ms));
}

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
static float pressure_to_altitude_m(float pressure_hpa, float sea_level_hpa)
{
    // Barometric formula (simple ISA approximation)
    // altitude = 44330 * (1 - (P/P0)^(1/5.255))
    if (pressure_hpa <= 0.0f || sea_level_hpa <= 0.0f) return 0.0f;
    return 44330.0f * (1.0f - powf(pressure_hpa / sea_level_hpa, 0.19029495f));
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------
esp_err_t bme680_init(void)
{
    memset(&g_dev, 0, sizeof(g_dev));

    static uint8_t addr = BME680_I2C_ADDR;

    g_dev.intf = BME68X_I2C_INTF;
    g_dev.read = user_i2c_read;
    g_dev.write = user_i2c_write;
    g_dev.delay_us = user_delay_us;
    g_dev.intf_ptr = &addr;

    int8_t rslt = bme68x_init(&g_dev);
    if (rslt != BME68X_OK)
    {
        ESP_LOGE(TAG, "bme68x_init failed: %d", rslt);
        g_ready = false;
        return ESP_FAIL;
    }

    // ---------------------------
    // Basic measurement config
    // ---------------------------
    struct bme68x_conf conf;
    memset(&conf, 0, sizeof(conf));

    conf.os_hum  = BME68X_OS_2X;
    conf.os_pres = BME68X_OS_4X;
    conf.os_temp = BME68X_OS_4X;
    conf.filter  = BME68X_FILTER_SIZE_3;
    conf.odr     = BME68X_ODR_NONE;          // forced mode: ODR not used

    rslt = bme68x_set_conf(&conf, &g_dev);
    if (rslt != BME68X_OK)
    {
        ESP_LOGE(TAG, "bme68x_set_conf failed: %d", rslt);
        g_ready = false;
        return ESP_FAIL;
    }

    // ---------------------------
    // Heater config (we can still enable it; you can disable gas later)
    // ---------------------------
    struct bme68x_heatr_conf heatr_conf;
    memset(&heatr_conf, 0, sizeof(heatr_conf));

    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;     // deg C
    heatr_conf.heatr_dur  = 100;     // ms

    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &g_dev);
    if (rslt != BME68X_OK)
    {
        ESP_LOGE(TAG, "bme68x_set_heatr_conf failed: %d", rslt);
        g_ready = false;
        return ESP_FAIL;
    }

    // Read once and set altitude offset so “altitude” starts near 0m at boot
    bme_data_t first = {0};
    esp_err_t e = bme680_read(&first);
    if (e == ESP_OK)
    {
        g_altitude_offset_m = first.altitude_m; // subtract this later
    }

    g_ready = true;
    ESP_LOGI(TAG, "BME680 init OK (addr=0x%02X)", BME680_I2C_ADDR);
    return ESP_OK;
}

esp_err_t bme680_read(bme_data_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;
    if (!g_ready) return ESP_ERR_INVALID_STATE;

    // Set forced mode
    int8_t rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &g_dev);
    if (rslt != BME68X_OK)
    {
        ESP_LOGW(TAG, "set_op_mode failed: %d", rslt);
        return ESP_FAIL;
    }

    // Wait for measurement to complete
    struct bme68x_conf conf;
    memset(&conf, 0, sizeof(conf));
    rslt = bme68x_get_conf(&conf, &g_dev);
    if (rslt != BME68X_OK)
    {
        ESP_LOGW(TAG, "get_conf failed: %d", rslt);
        return ESP_FAIL;
    }

    uint32_t dur_ms = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &g_dev);
    // Add heater duration + a little margin
    dur_ms += 120;

    vTaskDelay(pdMS_TO_TICKS(dur_ms));

    // Read data
    struct bme68x_data data;
    memset(&data, 0, sizeof(data));
    uint8_t n_fields = 0;

    rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &g_dev);
    if (rslt != BME68X_OK || n_fields == 0)
    {
        ESP_LOGW(TAG, "get_data failed: rslt=%d fields=%u", rslt, (unsigned)n_fields);
        return ESP_FAIL;
    }

    // Bosch returns:
    //  - temperature in °C (float)
    //  - pressure in Pa (float)
    //  - humidity in %RH (float)
    float temp_c   = data.temperature;
    float press_hpa = data.pressure / 100.0f;
    float hum_pct  = data.humidity;

    float alt_m = pressure_to_altitude_m(press_hpa, g_sea_level_hpa);
    alt_m -= g_altitude_offset_m;

    out->temp_C     = temp_c;
    out->press_hPa  = press_hpa;
    out->hum_pct    = hum_pct;
    out->altitude_m = alt_m;

    return ESP_OK;
}
