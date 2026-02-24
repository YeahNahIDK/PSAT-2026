#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

/* === SHARED BUSES === */
// I2C (BMP390, ICM-42670-P)
#define PIN_I2C_SDA             0
#define PIN_I2C_SCL             1

// SPI (SD Card, LoRa)
#define PIN_SPI_MOSI            10
#define PIN_SPI_MISO            5
#define PIN_SPI_SCK             20


/* === PERIPHERAL CHIP SELECTS (SPI) === */
#define PIN_LORA_CS             21
#define PIN_SD_CS               3


/* === INDIVIDUAL COMPONENTS === */
// GPS (UART)
#define PIN_GPS_TX              8
#define PIN_GPS_RX              7
#define UART_GPS                0

// LoRa Control
#define PIN_LORA_DIO0           13 
#define PIN_LORA_RST            RADIOLIB_NC            
#define PIN_LORA_DIO1           RADIOLIB_NC

#define LORA_FREQUENCY          915     // MHz
#define LORA_BANDWIDTH          125     // KHz
#define LORA_SF                 10      // Spreading Factor
#define LORA_CR                 5       // Coding Rate: 4/5
#define LORA_SYNC_WORD          0x12
#define LORA_POWER              14      // dBm
#define LORA_PREAMBLE           8
#define LORA_GAIN               0

// Buzzer
#define PIN_BUZZER              6

// Servo
#define PIN_SERVO               2

// IMU
#define ADDRESS_IMU             0x69


/* === SYSTEM DEFINITIONS === */
#define SERVO_STARTING_ANGLE    15
#define SERVO_ENDING_ANGLE      0
#define CALIBRATION_READINGS    10

#endif