/* === Dependencies === */
#include "esp_log.h"
#include "esp_system.h"  // For diagnostics
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>
#include "hardware_config.h"


/* === Component Drivers === */
#include "LoRaHandler.h"
#include "GPSDriver.h"
#include "BuzzerDriver.h"
#include "BMP390Driver.h"
#include "ICMDriver.h"
#include "ServoDriver.h"
#include "SdDriver.h"


/* === Component Creation === */
LoRaHandler lora;
SdDriver sd(PIN_SD_CS);
BuzzerDriver buzzer(PIN_BUZZER);
GPSDriver gps(UART_GPS, PIN_GPS_RX, PIN_GPS_TX);
ServoDriver servo(PIN_SERVO);
BMP390Driver altimeter;
ICMDriver imu(Wire, ADDRESS_IMU);

enum FlightState {
    PRE_LAUNCH,
    ASCENDING,
    DESCENDING,
    LANDED
};

struct TelemetryData {
    FlightState current_state = FLIGHT_STATE_INIT;
    float altitude = 0.0f;
    float max_altitude = 0.0f;
    float temperature = 0.0f;
    bool apogee = false;
    unsigned long landed_time = 0;
    unsigned long start_time = 0;
}; TelemetryData flight;


/* === Prototype Definition === */
/* Setup */
bool log_init_status(bool success, const char* device_name);
void sensor_test(char *sensor_data);

/* Loop */
bool stability_check();
bool apogee_detect();
void send_gps();
void sd_write_data();

void interval_sd(int time_interval);
void interval_gps(int time_interval);
void interval_buzzer(int time_interval, int beep_length);


void setup() {
    xTaskCreate(heartbeat_task, "heartbeat", 1024, NULL, 1, NULL);  // Diagnostics

    /* === Communication === */
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    /* === Hardware Initialisation === */
    buzzer.begin();
    servo.begin();
    servo.writeAngle(SERVO_STARTING_ANGLE);

    buzzer.force_beep(1, 50);

    log_init_status(lora.begin(), "LoRa");
    diagnose_boot_reason();  // Diagnostics
    log_init_status(gps.begin(), "GPS");
    bool altimeter_init = log_init_status(altimeter.begin(), "Altimeter");
    bool imu_init = log_init_status(imu.begin(), "IMU");
    log_init_status(sd.begin(SPI), "SD");

    if (log_init_status(sd.openLog("/flight_data.csv"), "SD Write")) {
        sd.logData("Time (ms), Altitude (m), Temp (°C), "
           "Accel_X (G), Accel_Y (G), Accel_Z (G), "
           "Gyro_X (dps), Gyro_Y (dps), Gyro_Z (dps), "
           "Latitude, Longitude\n");
        sd.save(); 
    }

    buzzer.force_beep(1, 50);
    delay(500);

    /* === Calibration === */
    altimeter.calibrate();
    imu.calibrateGyro();

    // Gets average sensor readings
    if (altimeter_init && imu_init) {
        char sensor_data[64];
        sensor_test(sensor_data);
        lora.send(sensor_data);
    }

    buzzer.force_beep(3, 50);
}


void loop() {
    gps.update();
    buzzer.update();    
    flight.altitude = altimeter.getAltitude();
    flight.temperature = altimeter.getTemperature();

    switch(flight.current_state) {
        case PRE_LAUNCH: {
            interval_gps(INTERVAL_VERY_SLOW);

            if (flight.altitude >= STATE_TRANSITION_ALT) {
                flight.current_state = ASCENDING;
                flight.start_time = millis();
            }
            break;
        }

        case ASCENDING: {
            interval_sd(INTERVAL_VERY_FAST);
            interval_gps(INTERVAL_SLOW);

            flight.apogee = apogee_detect();
            if (flight.apogee) {
                flight.current_state = DESCENDING;

                char apogee_result[64] = {0};
                sprintf(apogee_result, "APOGEE CONFIRMED AT %.2fm", flight.max_altitude);
                lora.send(apogee_result);
            }
            break;
        }

        case DESCENDING: {
            interval_sd(INTERVAL_FAST);
            interval_gps(INTERVAL_SLOW);
            if (flight.apogee - flight.altitude > SERVO_REL_ALTITUDE) {
                servo.writeAngle(SERVO_ENDING_ANGLE);
            }
            
            if (stability_check()) {
                flight.current_state = LANDED;
                flight.landed_time = millis();
                lora.send("LANDING CONFIRMED");
            }
            break;
        }

        case LANDED: {
            interval_gps(INTERVAL_VERY_SLOW);
            if (millis() - flight.landed_time >= BUZZER_RECOVERY_DELAY) {
                interval_buzzer(INTERVAL_SLOW, 500);
            }
            break;
        }
    }
}


/* === Setup Functions === */
bool log_init_status(bool success, const char* device_name) {
    char setup_results[64] = {0};

    if (success) {
        sprintf(setup_results, "%s Init Success!\n", device_name);
    } else {
        sprintf(setup_results, "%s Init Failed!\n", device_name);
    }

    lora.send(setup_results);
    delay(50);

    return success;
}


void sensor_test(char *sensor_data) {
    float altitude_sum = 0;
    float temperature_sum = 0;
    float acceleration_sum = 0;
    float angular_velocity_sum = 0;

    for (int i = 0; i < CALIBRATION_READINGS; i++) {
        IMUData imu_data = imu.getData();
        altitude_sum += altimeter.getAltitude();
        temperature_sum += altimeter.getTemperature();

        acceleration_sum += sqrt(imu_data.accX * imu_data.accX + imu_data.accY * imu_data.accY + imu_data.accZ + imu_data.accZ);
        angular_velocity_sum += sqrt(imu_data.gyrX * imu_data.gyrX + imu_data.gyrY * imu_data.gyrY + imu_data.gyrZ + imu_data.gyrZ);
    }

    float altitude_avg = altitude_sum / CALIBRATION_READINGS;
    float temperature_avg = temperature_sum / CALIBRATION_READINGS;
    float acceleration_avg = acceleration_sum / CALIBRATION_READINGS;
    float angular_velocity_avg = angular_velocity_sum / CALIBRATION_READINGS;

    sprintf(sensor_data, "Alt: %.2f, Temp: %.2f, Accel: %.2f, Ang: %.2f\n", 
        altitude_avg, temperature_avg, acceleration_avg, angular_velocity_avg);
}


/* === Loop Functions === */
bool stability_check() {
    static unsigned long lastCheckTime = 0;
    static float anchorAltitude = 0;        
    static unsigned long stableStartTime = 0;

    bool gpsMoving = gps.isValid() && (gps.tGps.speed.kmph() > GPS_STABLE_MAX_SPEED);

    if (stableStartTime == 0 || abs(flight.altitude - anchorAltitude) > STABILITY_MAX_DRIFT) {
        anchorAltitude = flight.altitude;
        stableStartTime = millis(); 
        
        if (gpsMoving) {
            stableStartTime = 0; 
        }
    }
    else {
        if (stableStartTime != 0 && (millis() - stableStartTime > STABILITY_DETECT_MS)) {
            return true;
        }
    }

    return false;
}


bool apogee_detect() {
    static unsigned long last_update = 0;
    static float prev_altitude = 0.0f;

    if (last_update == 0) {
        last_update = millis();
        prev_altitude = flight.altitude;
        flight.max_altitude = flight.altitude;
        return false;
    }

    unsigned long current_time = millis();
    unsigned long delta_time = current_time - last_update;
    if (delta_time == 0) return false;

    float delta_altitude = flight.altitude - prev_altitude;
    float velocity = delta_altitude / (delta_time / 1000.0f);

    static int below_max_count = 0;

    /* Santiy Check */
    if (abs(velocity) < APOGEE_MAX_VELOCITY) {
        /* Set Maximum Altitude */
        if (flight.altitude > flight.max_altitude) {
            flight.max_altitude = flight.altitude;
            below_max_count = 0;
        } else if ((flight.max_altitude - flight.altitude) >= APOGEE_DROP_THRESHOLD){
            below_max_count += 1;
        } else {
            below_max_count = 0;  // Handles jitter
        }

        /* Set Apogee */
        if (below_max_count >= APOGEE_DETECT_COUNT) {
            return true;
        }
    }

    prev_altitude = flight.altitude;
    last_update = current_time;

    return false;
}


void send_gps() {
    char gps_data[128] = {0}; 
    
    const char team_char = 'P'; 
    
    bool has_fix = gps.isValid();
    char fix_status = has_fix ? 'G' : 'N';

    int sats = gps.getSatellites();
    if (sats > 15) sats = 15;
    if (sats < 0) sats = 0;
    
    float lat = has_fix ? gps.getLatitude() : 0.0;
    float lon = has_fix ? gps.getLongitude() : 0.0;
    float alt = has_fix ? gps.getAltitude() : 0.0;

    // Format: #C XX:XX:XX UTC; Y; ZZ; -XXX.XXXXX,-XXX.XXXXX; XXXXX.Xm\n
    sprintf(gps_data, "#%c %02d:%02d:%02d UTC; %c; %02d; %.5f,%.5f; %.1fm\n", 
            team_char, 
            gps.getHour(), gps.getMinute(), gps.getSecond(), 
            fix_status, 
            sats, 
            lat, lon, 
            alt);

    lora.send(gps_data);
}


void sd_write_data() {
    char log_buffer[256];
    IMUData imu_data = imu.getData();
    snprintf(log_buffer, sizeof(log_buffer), 
            "%lu, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.6f, %.6f\n",
            millis() - flight.start_time,   // Time (ms)
            flight.altitude,                // Altitude (m)
            flight.temperature,             // Temp (°C)
            imu_data.accX,                  // Accel X (G)
            imu_data.accY,                  // Accel Y (G)
            imu_data.accZ,                  // Accel Z (G)
            imu_data.gyrX,                  // Gyro X (dps)
            imu_data.gyrY,                  // Gyro Y (dps)
            imu_data.gyrZ,                  // Gyro Z (dps)
            gps.getLatitude(),              // Latitude
            gps.getLongitude()              // Longitude
    );
    sd.logData(log_buffer);
}


void interval_sd(int time_interval) {
    static unsigned long last_write = 0;
    if (millis() - last_write > time_interval) {
        sd_write_data();
        last_write = millis();
    }
}


void interval_gps(int time_interval) {
    static unsigned long last_transmission = 0;
    if (millis() - last_transmission > time_interval) {
        send_gps();
        last_transmission = millis();
    }
}


void interval_buzzer(int time_interval, int beep_length) {
    static unsigned long last_trigger = 0;
    if (millis() - last_trigger > time_interval) {
        buzzer.beep(1, beep_length);
        last_trigger = millis();
    }
}


/* === Diagnostic Functions === */
void diagnose_boot_reason() {
    esp_reset_reason_t reason = esp_reset_reason();
    char reason_str[128] = {0};

    switch (reason) {
        case ESP_RST_POWERON:  
            sprintf(reason_str, "BOOT DIAG: Clean Power On\n"); 
            break;
        case ESP_RST_BROWNOUT: 
            sprintf(reason_str, "BOOT DIAG: BROWNOUT DETECTED (Power dropped during shock)\n"); 
            break;
        case ESP_RST_PANIC:    
            sprintf(reason_str, "BOOT DIAG: PANIC/CRASH (Software exception or bus lockup)\n"); 
            break;
        case ESP_RST_INT_WDT:
        case ESP_RST_TASK_WDT:
        case ESP_RST_WDT:      
            sprintf(reason_str, "BOOT DIAG: WATCHDOG RESET (Code froze)\n"); 
            break;
        default:               
            sprintf(reason_str, "BOOT DIAG: Other Reset (%d)\n", reason); 
            break;
    }

    lora.send(reason_str);
    delay(100); 
}

#define HEARTBEAT_LED_PIN 8
void heartbeat_task(void *pvParameter) {
    pinMode(HEARTBEAT_LED_PIN, OUTPUT);
    while(1) {
        digitalWrite(HEARTBEAT_LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(50)); // Short flash
        digitalWrite(HEARTBEAT_LED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(950)); // Wait a second
    }
}
