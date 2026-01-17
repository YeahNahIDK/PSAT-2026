/*
 * ============================================================================
 * LORA DRIVER - Implementation (Transparent UART Mode)
 * ============================================================================
 * 
 * Simple transparent mode: Send string, LoRa forwards it.
 * Messages end with '\n'.
 * 
 * Uses UART manager for all communication.
 */

#include "./inc/lora.h"
#include "./inc/uart_manager.h"
#include <string.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// LoRa uses UART_A0 on MSP430FR2433 dev board
#define LORA_UART_MODULE    UART_A0

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

bool lora_init(void) {
    /*
     * Initialize LoRa UART
     * 
     * MSP430FR2433 UCA0 default pins:
     * - P1.4 = UCA0TXD (MSP → LoRa RX)
     * - P1.5 = UCA0RXD (LoRa TX → MSP)
     */
    
    uart_init(LORA_UART_MODULE, LORA_BAUD_RATE);
    
    // Small delay for LoRa module to stabilize
    volatile uint32_t i;
    for (i = 0; i < 10000; i++);
    
    return true;
}

bool lora_send(const char* str) {
    /*
     * Send string via LoRa
     * Automatically appends '\n' to mark end of message
     */
    
    if (str == NULL) {
        return false;
    }
    
    uint16_t len = strlen(str);
    
    // Check length
    if (len >= LORA_MAX_MESSAGE_SIZE) {
        return false;  // Message too long
    }
    
    // Send string
    uart_write_string(LORA_UART_MODULE, str);
    
    // Send newline to mark end of message
    uart_write_byte(LORA_UART_MODULE, '\n');
    
    return true;
}

uint16_t lora_available(void) {
    /*
     * Check how many bytes are available to read
     */
    return uart_available(LORA_UART_MODULE);
}

bool lora_receive(char* buffer, uint16_t buffer_size) {
    /*
     * Receive message from LoRa
     * Reads until '\n' is found
     * 
     * Returns true if complete message received
     */
    
    if (buffer == NULL || buffer_size == 0) {
        return false;
    }
    
    // Read until newline
    uint16_t bytes_read = uart_read_until(LORA_UART_MODULE, 
                                           (uint8_t*)buffer, 
                                           buffer_size - 1, 
                                           '\n');
    
    if (bytes_read > 0) {
        // Remove trailing '\n' or '\r'
        while (bytes_read > 0 && 
               (buffer[bytes_read - 1] == '\n' || buffer[bytes_read - 1] == '\r')) {
            bytes_read--;
        }
        
        // Null terminate
        buffer[bytes_read] = '\0';
        
        return true;
    }
    
    return false;
}

void lora_sleep(void) {
    /*
     * Put LoRa to sleep
     * 
     * This is module-specific. Some modules support sleep commands.
     * For transparent mode, you may need to:
     * - Send specific command (check your module's manual)
     * - Toggle a sleep pin
     * - Or just do nothing if always-on is acceptable
     */
    
    // Implement based on your LoRa module's datasheet
}

void lora_wake(void) {
    /*
     * Wake LoRa from sleep
     * 
     * Usually just sending any character wakes it up
     */
    uart_write_byte(LORA_UART_MODULE, 0xFF);
}

/*
 * ============================================================================
 * USAGE EXAMPLE
 * ============================================================================
 * 
 * // In setup:
 * lora_init();
 * 
 * // Send a message:
 * lora_send("Tests started");
 * lora_send("Count: 42");
 * 
 * // Receive messages:
 * char msg[100];
 * if (lora_receive(msg, sizeof(msg))) {
 *     // msg contains the received message
 *     // Process msg...
 * }
 * 
 * // Or check if data available first:
 * if (lora_available() > 0) {
 *     if (lora_receive(msg, sizeof(msg))) {
 *         // Got complete message
 *     }
 * }
 */
