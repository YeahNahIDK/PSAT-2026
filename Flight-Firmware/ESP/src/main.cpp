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
#include "SdDriver.h"

static const char *TAG = "MAIN";

LoRaHandler lora;

#define SD_CS_PIN 3
SdDriver sd(SD_CS_PIN);

#define BUZZER_PIN  6
BuzzerDriver buzzer(BUZZER_PIN);

#define GPS_RX_PIN 7 // Wire to ESP TX
#define GPS_TX_PIN 8
#define GPS_UART 0
GPSDriver gps(GPS_UART, GPS_RX_PIN, GPS_TX_PIN);

#define SERVO_PIN 2
#define SERVO_STARTING_ANGLE 15
#define SERVO_ENDING_ANGLE 0
ServoDriver servo(SERVO_PIN);

BMP390Driver altimeter;
ICMDriver imu(Wire, 0x69);

// Timers for non-blocking delays
unsigned long lastTelemetryTime = 0;
const long TELEMETRY_INTERVAL = 1000;
unsigned long servoTime = 0;

#define APOGEE_THRESHOLD 10 // metres
bool APOGEE = false;
float alt_prev;
float alt_max = 0;

bool landing = false;

void imu_test() {
    char to_send[50] = {0};
    if (imu.update()) {
        IMUData d = imu.getData();
        sprintf(to_send, "A: %.2f %.2f %.2f | G: %.2f %.2f %.2f\n", 
            d.accX, d.accY, d.accZ, 
            d.gyrX, d.gyrY, d.gyrZ);
    }

    lora.send(to_send);
}

RecoveryState get_flight_state() {
    char to_send[50] = {0};
    // 1. GPS Data
    float gpsSpeed = gps.tGps.speed.kmph();
    bool gpsOk = gps.isValid();

    // 2. Apogee Flag 
    // (Placeholder: Logic will return immediately if this is false)
    bool apogeeReached = true; 

    // 3. PASS TO LOGIC
    // The function will call altimeter.getAltitude() internally.
    RecoveryState current_state = check_recovery_logic(apogeeReached, altimeter, gpsSpeed, gpsOk, buzzer);

    float debugAlt = altimeter.getAltitude();

    return current_state;
}

void send_gps() {
    char gps_data[50] = {0};
    bool has_fix = gps.isValid();

    if (has_fix) {
        sprintf(gps_data, "FIX, %.6f, %.6f\n", gps.getLatitude(), gps.getLongitude());
    } else {
        sprintf(gps_data, "NOFIX (Sats: %d)\n", gps.getSatellites());
    }

    lora.send(gps_data);
}

void detect_apogee() {
    float smoothing_factor = 0.1;

    float alt_curr = altimeter.getAltitude();
    float alt_average = alt_prev * smoothing_factor + alt_curr * (1-smoothing_factor);

    if (alt_average > alt_max) {
        alt_max = alt_average;
    }

    if (alt_average < alt_max - APOGEE_THRESHOLD) {
        APOGEE = true;
    }

    alt_prev = alt_average;
}

void setup() {
    // SCK, MISO, MOSI, SS
    // SPI.begin(19, 15, 18, 2); // C6
    SPI.begin(20, 5, 10, 21); // C3

    // SDA, SCL
    Wire.begin(0, 1);

    char setup_results[100] = {0};

    buzzer.begin();

    // delay(50);

    // buzzer.beep(1, 50);

    // delay(50);

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

    if (sd.begin(SPI)) { 
        // Open the file once
        if (sd.openLog("/flight_data.csv")) {
            // Write the header and manually force a save
            sd.logData("Time_ms,Altitude\n");
            sd.save(); 
            sprintf(setup_results, "Log opened and header written.");
        } else {
            sprintf(setup_results, "SD Could Not Open File");
        }
    } else {
        sprintf(setup_results, "SD Init Failed!");
    }
    lora.send(setup_results);
    delay(50);
    
    servo.begin();
    servo.writeAngle(SERVO_STARTING_ANGLE);
    
    delay(500);

    altimeter.calibrate();
    imu.calibrateGyro();

    /* Test readings */
    const int READINGS = 10;

    float alt_sum = 0;
    float temp_sum = 0;
    float acc_sum = 0;
    float ang_sum = 0;

    for (int i = 0; i < READINGS; i++) {
        IMUData d = imu.getData();
        alt_sum += altimeter.getAltitude();
        temp_sum += altimeter.getTemperature();

        acc_sum += sqrt(d.accX * d.accX + d.accY * d.accY + d.accZ + d.accZ);
        ang_sum += sqrt(d.gyrX * d.gyrX + d.gyrY * d.gyrY + d.gyrZ + d.gyrZ);
    }

    float alt_avg = alt_sum / READINGS;
    float temp_avg = temp_sum / READINGS;
    float acc_avg = acc_sum / READINGS;
    float ang_avg = ang_sum / READINGS;

    sprintf(setup_results, "Alt: %.2f, Temp: %.2f, Accel: %.2f, Ang: %.2f\n", alt_avg, temp_avg, acc_avg, ang_avg);
    lora.send(setup_results);

    alt_prev = altimeter.getAltitude();

    // buzzer.beep(1, 50);
    // delay(1000 * 60 * 2 + 30 * 1000);
    // lora.send("DROP DROP DROP\n");
}

void loop() {
    gps.update();
    buzzer.update();
    
    if (millis() - lastTelemetryTime > TELEMETRY_INTERVAL) {
        lastTelemetryTime = millis();
        
        // buzzer.beep(1, 500);

        RecoveryState current_state = get_flight_state();
        if (current_state == REC_IN_AIR) {
            detect_apogee();
        }

        if (current_state == REC_LANDED && !landing) {
            lora.send("LANDING CONFIRMED");
            landing = true;
            // buzzer.beep(1, 500);
        }

        send_gps();
        imu_test();

        char to_send[50] = {0};
        sprintf(to_send, "Alt: %.2f", altimeter.getAltitude());
        lora.send(to_send);

        // SD Test
        sd.logData("Hello World\n");
        sd.closeLog();
    }

    if (APOGEE || millis() - servoTime > 2*TELEMETRY_INTERVAL) { // SET SO GOES AFTER PARACHUTE DEPLOYMENT
        servoTime = millis();
        servo.writeAngle(SERVO_ENDING_ANGLE);
    }
}
