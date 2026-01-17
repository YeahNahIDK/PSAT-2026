/*
 * ============================================================================
 * MSP430FR2433 - GPS & LORA TEST PROGRAM
 * ============================================================================
 * 
 * Tests both GPS and LoRa communication
 * 
 * Hardware Setup:
 * - MSP430FR2433 Dev Board
 * - LoRa on UCA0 (P1.4=TX, P1.5=RX)
 * - GPS on UCA1 (P1.6=TX, P1.7=RX)
 * - LED on P1.0 (adjust if needed)
 * 
 * Behavior:
 * 1. Sends "Tests started" via LoRa
 * 2. Continuously reads GPS
 * 3. Every 5 seconds, sends GPS data via LoRa
 * 4. LED toggles each time data is sent
 * 5. Displays any received LoRa messages
 */

#include <msp430.h>
#include <msp430fr2433.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "./inc/uart_manager.h"
#include "./inc/lora.h"
#include "./inc/gps.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

// LED for visual feedback (on P1.0)
#define LED_PIN     BIT0

// Timing
#define GPS_SEND_INTERVAL_MS    5000    // Send GPS data every 5 seconds

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// System tick counter (incremented by timer interrupt every 1ms)
volatile uint32_t g_system_tick = 0;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

void system_init(void);
void clock_init(void);
void timer_init(void);
void led_init(void);
void led_toggle(void);
void delay_ms(uint32_t ms);
void format_gps_message(char* buffer, uint16_t size, const gps_data_t* gps);

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(void) {
    // Initialize system
    system_init();
    
    // Initialize LED
    led_init();
    
    // Blink LED 3 times to show we're starting
    for (uint8_t i = 0; i < 3; i++) {
        led_toggle();
        delay_ms(200);
        led_toggle();
        delay_ms(200);
    }
    
    // Initialize communication modules
    lora_init();
    gps_init();
    
    // Send startup message via LoRa
    lora_send("Tests started");
    delay_ms(100);
    
    // Variables for main loop
    uint32_t last_gps_send = 0;
    uint32_t message_count = 0;
    char lora_buffer[100];
    char gps_message[150];
    
    // Main loop
    while (1) {
        // ====================================================================
        // GPS PROCESSING
        // ====================================================================
        
        // Continuously read GPS (non-blocking)
        if (gps_read()) {
            // New GPS data parsed - could do something here if needed
        }
        
        // Send GPS data via LoRa every 5 seconds
        if ((g_system_tick - last_gps_send) >= GPS_SEND_INTERVAL_MS) {
            gps_data_t gps_data;
            
            if (gps_get_data(&gps_data)) {
                // We have valid GPS fix
                format_gps_message(gps_message, sizeof(gps_message), &gps_data);
                strcat(gps_message, "\n");
                lora_send(gps_message);
                
                // Toggle LED to show activity
                led_toggle();
            } else {
                // No GPS fix yet
                lora_send("GPS: No fix\n");
            }
            
            last_gps_send = g_system_tick;
            message_count++;
        }
        
        // ====================================================================
        // LORA RECEIVE PROCESSING
        // ====================================================================
        
        // Check for incoming LoRa messages
        if (lora_available() > 0) {
            if (lora_receive(lora_buffer, sizeof(lora_buffer))) {
                // Received a complete message
                // Echo it back with prefix (simple string concat)
                char echo[120];
                strcpy(echo, "RX:");
                strcat(echo, lora_buffer);
                lora_send(echo);
                
                // Toggle LED
                led_toggle();
            }
        }
        
        // ====================================================================
        // HEARTBEAT
        // ====================================================================
        
        // Send heartbeat every 30 seconds
        static uint32_t last_heartbeat = 0;
        if ((g_system_tick - last_heartbeat) >= 30000) {
            // Simple heartbeat message
            lora_send("HB\n");
            last_heartbeat = g_system_tick;
        }
    }
}

// ============================================================================
// SYSTEM INITIALIZATION
// ============================================================================

void system_init(void) {
    // Stop watchdog
    WDTCTL = WDTPW | WDTHOLD;
    
    // Initialize clocks
    clock_init();
    
    // Initialize timer
    timer_init();
}

void clock_init(void) {
    /*
     * Simple clock configuration for MSP430FR2433
     * Uses DCO at ~1MHz (default)
     * 
     * FR2433 has a simpler clock system than FR2355
     * We'll use the default DCO frequency
     */
    
    // Disable GPIO power-on default high-impedance
    PM5CTL0 &= ~LOCKLPM5;
    
    // Use default DCO (~1MHz)
    // No clock configuration needed - defaults are fine for testing
    // UART baud rate calculations in uart_manager.c will need adjustment
}

void timer_init(void) {
    /*
     * Configure Timer_A0 for 1ms interrupt
     * MSP430FR2433 has Timer_A, not Timer_B
     * With 1MHz SMCLK (no divider)
     * Count to 1000 = 1ms
     */
    TA0CCR0 = 1000 - 1;
    TA0CTL = TASSEL__SMCLK | MC__UP;  // SMCLK, no divider, UP mode
    TA0CCTL0 = CCIE;
    
    __enable_interrupt();
}

void led_init(void) {
    P1DIR |= LED_PIN;   // Set as output
    P1OUT &= ~LED_PIN;  // Turn off initially
}

void led_toggle(void) {
    P1OUT ^= LED_PIN;
}

void delay_ms(uint32_t ms) {
    uint32_t start = g_system_tick;
    while ((g_system_tick - start) < ms);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void format_gps_message(char* buffer, uint16_t size, const gps_data_t* gps) {
    /*
     * Format GPS data into a readable message
     * Uses simple string operations to avoid snprintf
     * 
     * Example output:
     * "GPS: -37.783300,175.266700,8sats,545m"
     */
    
    if (buffer == NULL || size < 50) return;
    
    // Start with prefix
    strcpy(buffer, "GPS:");
    uint16_t pos = 4;
    
    // Add latitude (simple conversion)
    int32_t lat_whole = (int32_t)gps->latitude;
    int32_t lat_frac = (int32_t)((gps->latitude - lat_whole) * 1000000);
    if (lat_frac < 0) lat_frac = -lat_frac;
    
    // Convert to string manually (avoid sprintf)
    char temp[20];
    int idx = 0;
    
    // Handle negative
    if (lat_whole < 0) {
        buffer[pos++] = '-';
        lat_whole = -lat_whole;
    }
    
    // Convert whole part
    if (lat_whole == 0) {
        buffer[pos++] = '0';
    } else {
        // Build number backwards
        int32_t n = lat_whole;
        idx = 0;
        while (n > 0) {
            temp[idx++] = '0' + (n % 10);
            n /= 10;
        }
        // Reverse into buffer
        while (idx > 0) {
            buffer[pos++] = temp[--idx];
        }
    }
    
    buffer[pos++] = '.';
    
    // Add 6 decimal places
    for (int i = 5; i >= 0; i--) {
        int32_t divisor = 1;
        for (int j = 0; j < i; j++) divisor *= 10;
        buffer[pos++] = '0' + ((lat_frac / divisor) % 10);
    }
    
    buffer[pos++] = ',';
    
    // Longitude (similar but simpler - just show as int)
    int32_t lon = (int32_t)gps->longitude;
    if (lon < 0) {
        buffer[pos++] = '-';
        lon = -lon;
    }
    if (lon > 100) buffer[pos++] = '0' + (lon / 100);
    if (lon > 10) buffer[pos++] = '0' + ((lon / 10) % 10);
    buffer[pos++] = '0' + (lon % 10);
    
    buffer[pos++] = ',';
    
    // Satellites
    if (gps->satellites > 9) buffer[pos++] = '0' + (gps->satellites / 10);
    buffer[pos++] = '0' + (gps->satellites % 10);
    buffer[pos++] = 's';
    buffer[pos++] = ',';
    
    // Altitude
    int32_t alt = (int32_t)gps->altitude;
    if (alt > 999) buffer[pos++] = '0' + (alt / 1000);
    if (alt > 99) buffer[pos++] = '0' + ((alt / 100) % 10);
    if (alt > 9) buffer[pos++] = '0' + ((alt / 10) % 10);
    buffer[pos++] = '0' + (alt % 10);
    buffer[pos++] = 'm';
    
    buffer[pos] = '\0';
}

// ============================================================================
// TIMER INTERRUPT
// ============================================================================

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
}

/*
 * ============================================================================
 * TESTING INSTRUCTIONS
 * ============================================================================
 * 
 * Hardware Setup:
 * 1. Connect LoRa module to UCA0 (P1.4=TX, P1.5=RX)
 * 2. Connect GPS module to UCA1 (P1.6=TX, P1.7=RX)
 * 3. Connect LED to P1.0 (or adjust LED_PIN)
 * 4. Power both modules with 3.3V
 * 
 * Expected Behavior:
 * 1. LED blinks 3 times on startup
 * 2. LoRa sends "Tests started"
 * 3. Every 5 seconds:
 *    - Sends GPS data via LoRa (if GPS has fix)
 *    - Or sends "GPS: No fix" (if no GPS lock)
 *    - LED toggles
 * 4. If LoRa message received:
 *    - Echoes it back with "Received: " prefix
 *    - LED toggles
 * 5. Every 30 seconds:
 *    - Sends heartbeat message
 * 
 * Testing with another device:
 * 1. Set up another MSP430 + LoRa with this same code
 * 2. Both will send GPS data every 5 seconds
 * 3. Each will receive and echo the other's messages
 * 4. Watch LEDs toggle on activity
 * 
 * Troubleshooting:
 * - No LED activity: Check power, clock configuration
 * - No LoRa messages: Check UART connections, baud rate
 * - GPS shows "No fix": Wait outdoors with clear sky view (can take 1-2 min)
 * - Wrong GPS data: Check UART connections (TX/RX might be swapped)
 */
