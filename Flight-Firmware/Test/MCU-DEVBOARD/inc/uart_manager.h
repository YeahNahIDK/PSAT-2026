/*
 * ============================================================================
 * UART MANAGER - Header File
 * ============================================================================
 * 
 * Supports multiple UART modules (UCA0, UCA1, etc.)
 * 
 * Features:
 * - Interrupt-driven RX with circular buffers
 * - Blocking and non-blocking TX
 * - Configurable baud rates
 */

#ifndef UART_MANAGER_H
#define UART_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// UART MODULE SELECTION
// ============================================================================

typedef enum {
    UART_A0 = 0,    // UCA0 - Debug/general purpose
    UART_A1 = 1,    // UCA1 - GPS
    UART_COUNT      // Total number of UART modules
} uart_module_t;

// ============================================================================
// CONFIGURATION
// ============================================================================

#define UART_RX_BUFFER_SIZE     128     // Size of RX circular buffer

// Common baud rates
#define BAUD_9600       9600
#define BAUD_19200      19200
#define BAUD_38400      38400
#define BAUD_57600      57600
#define BAUD_115200     115200

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

/**
 * Initialize UART module.
 * 
 * Parameters:
 *   module    - Which UART module to initialize (UART_A0, UART_A1, etc.)
 *   baud_rate - Baud rate (e.g., 9600, 115200)
 * 
 * Note: Pin configuration must match hardware
 * 
 * MSP430FR2355 defaults:
 * - UCA0: P1.7 (TX), P1.6 (RX)
 * - UCA1: P4.3 (TX), P4.2 (RX)
 * 
 * Example:
 *   uart_init(UART_A1, BAUD_9600);  // GPS at 9600 baud
 */
void uart_init(uart_module_t module, uint32_t baud_rate);

/**
 * Send a single byte (blocking).
 * 
 * Parameters:
 *   module - UART module to use
 *   data   - Byte to send
 * 
 * Example:
 *   uart_write_byte(UART_A0, 'A');
 */
void uart_write_byte(uart_module_t module, uint8_t data);

/**
 * Send a string (blocking).
 * 
 * Parameters:
 *   module - UART module to use
 *   str    - Null-terminated string to send
 * 
 * Example:
 *   uart_write_string(UART_A0, "Hello World\r\n");
 */
void uart_write_string(uart_module_t module, const char* str);

/**
 * Send a buffer of bytes (blocking).
 * 
 * Parameters:
 *   module - UART module to use
 *   buffer - Data to send
 *   length - Number of bytes to send
 * 
 * Example:
 *   uint8_t data[] = {0x01, 0x02, 0x03};
 *   uart_write_buffer(UART_A0, data, 3);
 */
void uart_write_buffer(uart_module_t module, const uint8_t* buffer, uint16_t length);

/**
 * Check if data is available to read.
 * 
 * Parameters:
 *   module - UART module to check
 * 
 * Returns:
 *   Number of bytes available in RX buffer
 * 
 * Example:
 *   if (uart_available(UART_A1) > 0) {
 *       // Data available
 *   }
 */
uint16_t uart_available(uart_module_t module);

/**
 * Read a single byte (non-blocking).
 * 
 * Parameters:
 *   module - UART module to read from
 *   data   - Pointer to store received byte
 * 
 * Returns:
 *   true  - Byte read successfully
 *   false - No data available
 * 
 * Example:
 *   uint8_t byte;
 *   if (uart_read_byte(UART_A1, &byte)) {
 *       // Process byte
 *   }
 */
bool uart_read_byte(uart_module_t module, uint8_t* data);

/**
 * Read multiple bytes (non-blocking).
 * 
 * Parameters:
 *   module      - UART module to read from
 *   buffer      - Buffer to store received data
 *   max_length  - Maximum bytes to read
 * 
 * Returns:
 *   Number of bytes actually read
 * 
 * Example:
 *   uint8_t buffer[32];
 *   uint16_t received = uart_read_buffer(UART_A1, buffer, 32);
 */
uint16_t uart_read_buffer(uart_module_t module, uint8_t* buffer, uint16_t max_length);

/**
 * Peek at next byte without removing it from buffer.
 * 
 * Parameters:
 *   module - UART module to peek
 *   data   - Pointer to store peeked byte
 * 
 * Returns:
 *   true  - Byte available and stored
 *   false - No data available
 * 
 * Example:
 *   uint8_t next_byte;
 *   if (uart_peek(UART_A1, &next_byte)) {
 *       if (next_byte == '$') {
 *           // Start of NMEA sentence
 *       }
 *   }
 */
bool uart_peek(uart_module_t module, uint8_t* data);

/**
 * Clear RX buffer (discard all received data).
 * 
 * Parameters:
 *   module - UART module to clear
 * 
 * Example:
 *   uart_flush(UART_A1);
 */
void uart_flush(uart_module_t module);

/**
 * Read until specific character or timeout.
 * 
 * Parameters:
 *   module      - UART module to read from
 *   buffer      - Buffer to store data
 *   max_length  - Maximum bytes to read
 *   terminator  - Stop when this character is found
 * 
 * Returns:
 *   Number of bytes read (including terminator if found)
 * 
 * Example:
 *   char line[128];
 *   uint16_t len = uart_read_until(UART_A1, line, 128, '\n');
 */
uint16_t uart_read_until(uart_module_t module, uint8_t* buffer, uint16_t max_length, uint8_t terminator);

#endif // UART_MANAGER_H
