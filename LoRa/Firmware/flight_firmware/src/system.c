// system.c — IMPLEMENTATION OF HARDWARE SETUP 

// NOTE: This file doesn't contain real register names yet.
// It only explains what needs to happen, not how on MSP430.


static unsigned long system_time_ms = 0;

void system_clock_init() {
    // Set MCU clock speed (e.g., 16 MHz or 48 MHz depending on chip not sure off the top of my head).
    // Configure PLL or DCO.
}

void system_uart_init() {
    // UART1 = GPS     @ 9600 baud
    // UART2 = LoRa    @ 115200 baud
    // UART3 = IMU     @ 115200 baud (BNO085 RVC mode)
}

void system_i2c_init() {
    // Set pins to SDA/SCL
    // Configure I2C to 400 kHz mode
}

void system_spi_init() {
    // SPI for SD card reader (DFR0229):
    // MISO = 12, MOSI = 11, SCK = 13 (from module diagram)
    // CS pin manually controlled
}

void system_timer_init() {
    // Setup timer interrupt to fire every 1 millisecond.
    // Inside interrupt: system_time_ms++;
}

unsigned long millis() {
    return system_time_ms;
}

void delay_ms(int ms) {
    // wait until millis() increases by ms
    // we prolly shouldnt be using actual delay functions
}
