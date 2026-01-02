// system.h — LOW-LEVEL HARDWARE INITIALISATION

void system_clock_init();       // Setup MCU clock
void system_uart_init();        // Setup UART ports (GPS, IMU, LoRa)
void system_i2c_init();         // For BMP388 if using I2C
void system_spi_init();         // For SD card module
void system_timer_init();       // Millisecond tick
uint32_t millis();              // Returns system uptime in ms
