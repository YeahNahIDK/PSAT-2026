/*
 * ============================================================================
 * MSP430 LORA TEST PROGRAM
 * ============================================================================
 * 
 * Goal: Test LoRa communication
 * 
 * This program:
 * 1. Initializes SPI and LoRa
 * 2. Sends "Tests started" message
 * 3. Sends a counter every second
 * 4. Blinks LED on successful transmission
 */

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "spi_manager.h"
#include "lora.h"

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// System tick counter (incremented by timer interrupt every 1ms)
volatile uint32_t g_system_tick = 0;

// ============================================================================
// LED CONTROL (for visual feedback)
// ============================================================================

// Assuming LED on P2.0 (adjust based on your hardware)
#define LED_PORT    P2OUT
#define LED_DIR     P2DIR
#define LED_PIN     BIT0

void led_init(void) {
    LED_DIR |= LED_PIN;   // Set as output
    LED_PORT &= ~LED_PIN; // Turn off initially
}

void led_toggle(void) {
    LED_PORT ^= LED_PIN;  // Toggle LED
}

// ============================================================================
// DELAY FUNCTION
// ============================================================================

void delay_ms(uint32_t milliseconds) {
    uint32_t start = g_system_tick;
    while ((g_system_tick - start) < milliseconds) {
        // Wait
    }
}

// ============================================================================
// TIMER INITIALIZATION
// ============================================================================

void timer_init(void) {
    /*
     * Configure Timer_A0 to interrupt every 1ms
     * This increments g_system_tick
     * 
     * Assuming 8MHz SMCLK (from your Rust config):
     * - Use SMCLK / 8 = 1MHz
     * - Count to 1000 = 1ms
     */
    TA0CCR0 = 1000 - 1;              // 1ms period at 1MHz
    TA0CTL = TASSEL__SMCLK |         // Use SMCLK
             ID__8 |                  // Divide by 8
             MC__UP;                  // Up mode
    TA0CCTL0 = CCIE;                 // Enable interrupt
    
    __enable_interrupt();             // Enable global interrupts
}

// ============================================================================
// CLOCK INITIALIZATION
// ============================================================================

void clock_init(void) {
    /*
     * Configure clocks to match your Rust setup:
     * - MCLK: 8MHz (DCO)
     * - SMCLK: 8MHz
     * 
     * This is simplified - adjust for your needs
     */
    
    // Disable watchdog
    WDTCTL = WDTPW | WDTHOLD;
    
    // Configure PMM
    PM5CTL0 &= ~LOCKLPM5;  // Disable GPIO power-on default high-impedance
    
    // Set DCO to 8MHz
    CSCTL0_H = CSKEY_H;                // Unlock CS registers
    CSCTL1 = DCOFSEL_3;                // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK |            // ACLK = VLO
             SELS__DCOCLK |            // SMCLK = DCO
             SELM__DCOCLK;             // MCLK = DCO
    CSCTL3 = DIVA__1 |                 // ACLK divider = 1
             DIVS__1 |                 // SMCLK divider = 1
             DIVM__1;                  // MCLK divider = 1
    CSCTL0_H = 0;                      // Lock CS registers
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(void) {
    // Initialize hardware
    clock_init();
    timer_init();
    led_init();
    
    // Initialize SPI (for LoRa)
    spi_init();
    
    // Small delay to let hardware settle
    delay_ms(100);
    
    // Initialize LoRa
    if (!lora_init()) {
        // LoRa initialization failed!
        // Blink LED rapidly to indicate error
        while (1) {
            led_toggle();
            delay_ms(100);
        }
    }
    
    // LoRa initialized successfully!
    // Blink LED 3 times to confirm
    for (uint8_t i = 0; i < 3; i++) {
        led_toggle();
        delay_ms(200);
        led_toggle();
        delay_ms(200);
    }
    
    // Send test message
    const char* test_msg = "Tests started\n";
    lora_send((uint8_t*)test_msg, strlen(test_msg));
    
    // Counter for messages
    uint32_t counter = 0;
    uint32_t last_send = 0;
    
    // Main loop - send counter every second
    while (1) {
        // Send message every 1000ms
        if ((g_system_tick - last_send) >= 1000) {
            // Format message
            char msg[32];
            snprintf(msg, sizeof(msg), "Count: %lu\n", counter);
            
            // Send via LoRa
            if (lora_send((uint8_t*)msg, strlen(msg))) {
                // Success - toggle LED
                led_toggle();
                counter++;
            }
            
            last_send = g_system_tick;
        }
    }
    
    return 0;
}

// ============================================================================
// TIMER INTERRUPT SERVICE ROUTINE
// ============================================================================

/*
 * This ISR runs every 1ms and increments g_system_tick
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    g_system_tick++;
    // CCR0 interrupt flag is cleared automatically
}

/*
 * ============================================================================
 * HOW TO TEST
 * ============================================================================
 * 
 * Hardware needed:
 * - MSP430FR2355 board
 * - Beacon board with LoRa module
 * - Antenna connected to LoRa
 * - LED on P2.0 (or adjust LED_PIN)
 * - (Optional) Another LoRa device to receive messages
 * 
 * Expected behavior:
 * 1. LED blinks 3 times on successful LoRa init
 * 2. Sends "Tests started"
 * 3. LED toggles every second as it sends counter
 * 4. If init fails, LED blinks rapidly forever
 * 
 * To receive messages:
 * - Use another MSP430 + LoRa running receive code
 * - Or use a LoRa USB dongle with 915MHz, SF10, BW62.5kHz
 * 
 * Troubleshooting:
 * - Rapid LED blink = LoRa init failed
 *   → Check SPI connections
 *   → Check LoRa power
 *   → Verify CS pin is correct (P4.4)
 * 
 * - No LED activity = Timer not working
 *   → Check clock configuration
 *   → Verify interrupts enabled
 * 
 * - LED toggles but no messages received elsewhere:
 *   → Check antenna connection
 *   → Verify receiver has same settings (915MHz, SF10, BW62.5kHz)
 *   → Check transmit power
 */