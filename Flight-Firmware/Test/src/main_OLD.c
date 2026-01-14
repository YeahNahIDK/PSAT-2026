#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "lora.h"
#include "gps.h"
#include "barometer.h"
#include "imu.h"
#include "sdcard.h"
#include "servo.h"
#include "buzzer.h"
#include "spi_manager.h"
#include "uart.h"

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void setup(void);
void loop(void);

// ============================================================================
// FUNCTION DEFINITIONS
// ============================================================================

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
    
    setup();

    while(1) {
        loop();
    }
}

void setup(void) {
    // Initialize communication interfaces
    spi_init();
    uart_init();

    // Initialize hardware
    lora_init();
    gps_init();
    baro_init();
    imu_init();
    sdcard_init();
    servo_init();
    buzzer_init();

    // Run pre-flight tests
    lora_send("Testing started\n");

    // Check sensor data
    gps_read();
    imu_read();
    baro_update();

    lora_send(buffer);

    // Check buzzer output
    buzzer_on();
    // Wait 5s
    buzzer_off();
    
    // Confirm tests are complete
    lora_send("Testing ended\n");
}

void loop(void) {
    // TODO: Read sensors
    // TODO: Transmit data
    // TODO: Log to SD card
    // TODO: Control outputs
}