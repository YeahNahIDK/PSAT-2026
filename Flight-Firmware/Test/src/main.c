#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void setup(void);
void loop(void);

// ============================================================================
// FUNCTION DEFINITIONS
// ============================================================================

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stops watchdog timer
    
    setup();

    while(1) {
        loop();
    }
}

void setup(void) {
    // TODO: Initialize hardware
    // TODO: Initialize communication interfaces
    // TODO: Initialize sensors
    // TODO: Run pre-flight tests
}

void loop(void) {
    // TODO: Read sensors
    // TODO: Transmit data
    // TODO: Log to SD card
    // TODO: Control outputs
}