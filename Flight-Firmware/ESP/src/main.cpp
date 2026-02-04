#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

#include "LoRaHandler.h"
#include "GPSDriver.h"
#include "BuzzerDriver.h"
#include "Recovery.h"
#include "BMP390Driver.h"

static const char *TAG = "MAIN";

LoRaHandler lora;

#define BUZZER_PIN  21
BuzzerDriver buzzer(BUZZER_PIN);

#define GPS_RX_PIN 5 // Wire to ESP TX
#define GPS_TX_PIN 4
#define GPS_UART 1
GPSDriver gps(GPS_UART, GPS_RX_PIN, GPS_TX_PIN);

BMP390Driver altimeter;

// Timers for non-blocking delays
unsigned long lastTelemetryTime = 0;
const long TELEMETRY_INTERVAL = 1000;

void imu_test() {}

void get_flight_state() {
    // 1. GPS Data
    float gpsSpeed = gps.tGps.speed.kmph();
    bool gpsOk = gps.isValid();

    // 2. Apogee Flag 
    // (Placeholder: Logic will return immediately if this is false)
    bool apogeeReached = true; 

    // 3. PASS TO LOGIC
    // The function will call altimeter.getAltitude() internally.
    check_recovery_logic(apogeeReached, altimeter, gpsSpeed, gpsOk, buzzer);

    float debugAlt = altimeter.getAltitude();
    ESP_LOGI(TAG, "BMP390 Alt: %.2fm  GPS Speed: %.2f", debugAlt, gpsSpeed);
}

void send_gps() {
    char gps_data[50] = {0};
    bool has_fix = false;

    if (gps.isValid()) {
            ESP_LOGI(TAG, "GPS: %.6f, %.6f\n", gps.getLatitude(), gps.getLongitude());
            has_fix = true;
    } else {
            ESP_LOGE(TAG, "GPS: No Fix (Sats: %d)\n", gps.getSatellites());
    }
    
    if (has_fix) {
        sprintf(gps_data, "FIX, %.6f, %.6f\n", gps.getLatitude(), gps.getLongitude());
    } else {
        sprintf(gps_data, "NOFIX\n");
    }
    
    lora.send(gps_data);
}

void setup() {
    Serial.begin(115200);
    
    Serial.println("Booting...");

    // For c6: SCK=18, MISO=20, MOSI=19, SS=9
    SPI.begin(18, 20, 19, 9);

    // SDA, SCL
    Wire.begin(6, 7);

    if (!lora.init()) {
        ESP_LOGE(TAG, "LoRa Init Failed!\n");
    }

    if (!gps.begin()) {
        ESP_LOGE(TAG, "GPS Init Failed!\n");
    }

    if (!altimeter.begin()) {
        ESP_LOGE(TAG, "BMP390 Init Failed! Check address");
    }
    
    buzzer.begin();
    
    delay(1000);

    altimeter.calibrate();
}

void loop() {
    gps.update();
    buzzer.update();
    
    if (millis() - lastTelemetryTime > TELEMETRY_INTERVAL) {
        lastTelemetryTime = millis();
        
        buzzer.beep(1, 500);

        get_flight_state();
        send_gps();

        Serial.println(altimeter.getTemperature());
    }
}
