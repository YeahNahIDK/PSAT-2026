#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

#include "LoRaHandler.h"
#include "GPSDriver.h"
#include "BuzzerDriver.h"
#include "Recovery.h"
#include "BMP390Driver.h"
#include "ICMDriver.h"
#include "ServoDriver.h"

static const char *TAG = "MAIN";

LoRaHandler lora;

#define BUZZER_PIN  6
BuzzerDriver buzzer(BUZZER_PIN);

#define GPS_RX_PIN 7 // Wire to ESP TX
#define GPS_TX_PIN 8
#define GPS_UART 0
GPSDriver gps(GPS_UART, GPS_RX_PIN, GPS_TX_PIN);

#define SERVO_PIN 2
ServoDriver servo(SERVO_PIN);

BMP390Driver altimeter;
ICMDriver imu(Wire, 0x69);

// Timers for non-blocking delays
unsigned long lastTelemetryTime = 0;
const long TELEMETRY_INTERVAL = 1000;

void imu_test() {
    char to_send[50] = {0};
    if (imu.update()) {
        IMUData d = imu.getData();
        sprintf(to_send, "A: %.2f %.2f %.2f | G: %.2f %.2f %.2f\n", 
            d.accX, d.accY, d.accZ, 
            d.gyrX, d.gyrY, d.gyrZ);

        lora.send(to_send);
    }
}

void get_flight_state() {
    char to_send[50] = {0};
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

    sprintf(to_send, "Alt: %.2fm, Temp: %.2f, GPS Speed: %.2f", debugAlt, altimeter.getTemperature(), gpsSpeed);
    lora.send(to_send);
}

void send_gps() {
    char gps_data[50] = {0};
    bool has_fix = false;

    if (has_fix) {
        sprintf(gps_data, "FIX, %.6f, %.6f\n", gps.getLatitude(), gps.getLongitude());
    } else {
        sprintf(gps_data, "NOFIX (Sats: %d)\n", gps.getSatellites());
    }
    
    lora.send(gps_data);
}

void setup() {
    // SCK, MISO, MOSI, SS
    // SPI.begin(19, 15, 18, 2); // C6
    SPI.begin(20, 5, 10, 21); // C3

    // SDA, SCL
    Wire.begin(0, 1);

    char setup_results[50] = {0};

    if (!lora.init()) {
        sprintf(setup_results, "LoRa Init Failed!\n");
    } else {
        sprintf(setup_results, "LoRa Init Success!\n");
    }
    lora.send(setup_results);
    delay(50);

    if (!gps.begin()) {
        sprintf(setup_results, "GPS Init Failed!\n");
    } else {
        sprintf(setup_results, "GPS Init Success!\n");
    }
    lora.send(setup_results);
    delay(50);

    if (!altimeter.begin()) {
        sprintf(setup_results, "BMP390 Init Failed!");
    } else {
        sprintf(setup_results, "BMP390 Init Success!\n");
    }
    lora.send(setup_results);
    delay(50);

    if (!imu.begin()) {
         sprintf(setup_results, "IMU Init Failed!");
    } else {
        sprintf(setup_results, "IMU Init Success!\n");
    }
    lora.send(setup_results);
    delay(50);
    
    buzzer.begin();
    servo.begin();
    
    delay(500);

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
        imu_test();
    }
}
