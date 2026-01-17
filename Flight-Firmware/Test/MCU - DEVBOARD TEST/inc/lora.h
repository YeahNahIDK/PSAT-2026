/*
 * ============================================================================
 * LORA DRIVER - Header File (Transparent UART Mode)
 * ============================================================================
 * 
 * Simple driver for LoRa module in transparent mode.
 * Just forwards UART data over LoRa radio.
 * 
 * Messages are terminated with '\n' (newline).
 * 
 * Configuration for MSP430FR2433:
 * - LoRa on UCA0 (UART0)
 * - GPS on UCA1 (UART1)
 */

#ifndef LORA_H
#define LORA_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// CONSTANTS
// ============================================================================

#define LORA_MAX_MESSAGE_SIZE   255     // Maximum message size
#define LORA_BAUD_RATE          9600    // LoRa UART baud rate (adjust if needed)

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

/**
 * Initialize LoRa module.
 * 
 * Configures UART for LoRa communication.
 * Uses UCA0 on MSP430FR2433.
 * 
 * Returns:
 *   true  - Initialization successful
 *   false - Initialization failed
 */
bool lora_init(void);

/**
 * Send string via LoRa.
 * Automatically appends '\n' to mark end of message.
 * 
 * Parameters:
 *   str - Null-terminated string to send
 * 
 * Returns:
 *   true  - Message sent successfully
 *   false - Message too long or error
 * 
 * Example:
 *   lora_send("Hello World");  // Sends "Hello World\n"
 */
bool lora_send(const char* str);

/**
 * Check if data has been received.
 * 
 * Returns:
 *   Number of bytes available to read
 */
uint16_t lora_available(void);

/**
 * Receive a message from LoRa (non-blocking).
 * Reads until '\n' is found.
 * 
 * Parameters:
 *   buffer      - Buffer to store received message
 *   buffer_size - Size of buffer
 * 
 * Returns:
 *   true  - Complete message received (terminated by '\n')
 *   false - No complete message yet
 * 
 * Example:
 *   char msg[100];
 *   if (lora_receive(msg, sizeof(msg))) {
 *       // msg contains received message (without '\n')
 *   }
 */
bool lora_receive(char* buffer, uint16_t buffer_size);

/**
 * Put LoRa module to sleep (if supported by your module).
 * 
 * Note: This may require specific commands depending on your LoRa module.
 * Implement based on your module's datasheet.
 */
void lora_sleep(void);

/**
 * Wake LoRa module from sleep.
 */
void lora_wake(void);

#endif // LORA_H
