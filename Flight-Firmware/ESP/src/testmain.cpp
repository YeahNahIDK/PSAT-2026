/* === Dependencies === */
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>
#include "hardware_config.h"


/* === Component Drivers === */
#include "LoRaHandler.h"
#include "GPSDriver.h"
#include "BuzzerDriver.h"
#include "Recovery.h"
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


/* === Prototype Definition === */
bool log_init_status(bool success, const char* device_name);
void sensor_test(char *sensor_data);


void setup() {
    /* === Communication === */
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_LORA_CS);
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    /* === Hardware Initialisation === */
    buzzer.begin();
    servo.begin();
    servo.writeAngle(SERVO_STARTING_ANGLE);

    buzzer.beep(1, 50);

    log_init_status(lora.begin(), "LoRa");
    log_init_status(gps.begin(), "GPS");
    bool altimeter_init = log_init_status(altimeter.begin(), "Altimeter");
    bool imu_init = log_init_status(imu.begin(), "IMU");
    log_init_status(sd.begin(SPI), "SD");

    if (log_init_status(sd.openLog("/flight_data.csv"), "SD Write")) {
        // Write the header and manually force a save
        sd.logData("Time_ms,Altitude\n");
        sd.save(); 
    }

    buzzer.beep(1, 50);
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
    
    buzzer.beep(3, 50);
}


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
