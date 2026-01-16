#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "./inc/spi_manager.h"
#include "./inc/lora.h"
#include "./inc/uart_manager.h"

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// System tick counter (incremented by timer interrupt every 1ms)
volatile uint32_t g_system_tick = 0;

// ============================================================================
// LED CONTROL (for visual feedback)
// ============================================================================

// Assuming LED on P2.0
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
     * With 8MHz SMCLK:
     * - Use SMCLK / 8 = 1MHz
     * - Count to 1000 = 1ms
     */
    TB0CCR0 = 1000 - 1;              // 1ms period at 1MHz
    TB0CTL = TBSSEL__SMCLK |         // Use SMCLK
             ID__8 |                 // Divide by 8
             MC__UP;                 // Up mode
    TB0CCTL0 = CCIE;                 // Enable interrupt
    
    __enable_interrupt();            // Enable global interrupts (GIE bit)
}

// ============================================================================
// CLOCK INITIALIZATION
// ============================================================================

void clock_init(void) {
    /*
     * Configure clocks:
     * - MCLK: 8MHz (DCO)
     * - SMCLK: 8MHz
     */
    
    // Disable watchdog
    WDTCTL = WDTPW | WDTHOLD;
    
    // Disable GPIO power-on default high-impedance
    PM5CTL0 &= ~LOCKLPM5;
    
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;
    
    // Clock System Setup
    // Per Device Errata set divider to 4 before changing frequency to
    // prevent out of spec operation from overshoot transient
    __bis_SR_register(SCG0);                // Disable FLL
    CSCTL3 |= SELREF__REFOCLK;              // Set REFO as FLL reference source
    CSCTL0 = 0;                             // Clear DCO settings
    CSCTL1 &= ~(DCORSEL_7);                 // Clear DCO frequency select bits
    CSCTL1 |= DCORSEL_3;                    // Set DCO = 8MHz
    CSCTL2 = FLLD_0 + 243;                  // DCOCLKDIV = 8MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                // Enable FLL
    
    // Wait for FLL to stabilize
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));
    
    // Select clock sources
    CSCTL4 = SELMS__DCOCLKDIV |             // MCLK = DCOCLKDIV (8MHz)
             SELA__REFOCLK;                  // ACLK = REFO (32768 Hz)
    
    // Set all dividers to 1
    CSCTL5 = DIVS__1 |                      // SMCLK divider = 1
             DIVM__1 |                      // MCLK divider = 1  
             DIVA__1;                        // ACLK divider = 1
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

    // TESTING FOR CDR:
    uart_init(UART_A0, BAUD_9600);
    
    // Main loop - send counter every second
    while (1) {
        // Send message every 1000ms
        if ((g_system_tick - last_send) >= 1000) {
            // Simple message without printf
            char msg[20];
            
            // Manual conversion of counter to string
            uint8_t len = 0;
            msg[len++] = 'C';
            msg[len++] = 'o';
            msg[len++] = 'u';
            msg[len++] = 'n';
            msg[len++] = 't';
            msg[len++] = ':';
            msg[len++] = ' ';
            
            // Convert number to ASCII (simple method for small numbers)
            if (counter >= 100) msg[len++] = '0' + (counter / 100) % 10;
            if (counter >= 10) msg[len++] = '0' + (counter / 10) % 10;
            msg[len++] = '0' + counter % 10;
            msg[len++] = '\n';
            msg[len] = '\0';
            
            // Send via LoRa
            if (lora_send((uint8_t*)msg, len)) {
                // Success - toggle LED
                led_toggle();
                counter++;
                if (counter > 999) counter = 0;  // Reset after 999
            }
            
            last_send = g_system_tick;
        }

        // TESTING FOR CDR
        uart_write_string(UART_A0, "PING\r\n");

        delay_ms(500);
    }
}

// ============================================================================
// TIMER INTERRUPT SERVICE ROUTINE
// ============================================================================

/*
 * This ISR runs every 1ms and increments g_system_tick
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B0_VECTOR))) Timer_B0_ISR (void)
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
