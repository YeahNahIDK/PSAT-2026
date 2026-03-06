#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

/* === SHARED BUSES === */
// I2C (BMP390, ICM-42670-P)
#define PIN_I2C_SDA                 0
#define PIN_I2C_SCL                 1

// SPI (SD Card, LoRa)
#define PIN_SPI_MOSI                10
#define PIN_SPI_MISO                5
#define PIN_SPI_SCK                 20


/* === PERIPHERAL CHIP SELECTS (SPI) === */
#define PIN_LORA_CS                 21
#define PIN_SD_CS                   3   


/* === INDIVIDUAL COMPONENTS === */
// GPS
#define PIN_GPS_TX                  8
#define PIN_GPS_RX                  7
#define UART_GPS                    0
#define GPS_STABLE_MAX_SPEED        15          // Km/h

// LoRa
#define PIN_LORA_DIO0               4 
#define PIN_LORA_RST                RADIOLIB_NC            
#define PIN_LORA_DIO1               RADIOLIB_NC

#define LORA_FREQUENCY              915         // MHz
#define LORA_BANDWIDTH              125         // KHz
#define LORA_SF                     8           // Spreading Factor
#define LORA_CR                     5           // Coding Rate: 4/5
#define LORA_SYNC_WORD              0x12
#define LORA_POWER                  14          // dBm
#define LORA_PREAMBLE               8
#define LORA_GAIN                   0

// Buzzer
#define PIN_BUZZER                  6
#define BUZZER_RECOVERY_DELAY       (15 * 60 * 1000)

// Servo
#define PIN_SERVO                   2

#define SERVO_MIN_ANGLE             0
#define SERVO_MIN_ANGLE_US          500
#define SERVO_MAX_ANGLE             180
#define SERVO_MAX_ANGLE_US          2500

// IMU
#define ADDRESS_IMU                 0x69

// Altimeter
#define ADDRESS_ALTIMETER           0x77
#define ADDRESS_BACKUP_ALTIMETER    0x76

/* === SYSTEM DEFINITIONS === */
#define SERVO_STARTING_ANGLE        108
#define SERVO_ENDING_ANGLE          88
#define SERVO_REL_ALTITUDE          150         // How far below apogee (m)
#define CALIBRATION_READINGS        10
#define FLIGHT_STATE_INIT           PRE_LAUNCH
#define STATE_TRANSITION_ALT        15          // m
#define STABILITY_MAX_DRIFT         2           // m
#define STABILITY_DETECT_MS         (10 * 1000)
#define APOGEE_MAX_VELOCITY         210         // ms⁻¹
#define APOGEE_DETECT_COUNT         5
#define APOGEE_DROP_THRESHOLD       4
#define SD_DATA_SAVE_INTERVAL       25          // Number of writes

/* Polling rates (ms) */
#define INTERVAL_VERY_SLOW          5000
#define INTERVAL_SLOW               1000
#define INTERVAL_FAST               500
#define INTERVAL_VERY_FAST          100

#endif