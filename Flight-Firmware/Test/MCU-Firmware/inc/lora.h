/*
 * ============================================================================
 * LORA DRIVER - Header File
 * ============================================================================
 * 
 * 
 * Configuration:
 * - Frequency: 915 MHz
 * - Bandwidth: 62.5 kHz
 * - Spreading Factor: 10
 * - Coding Rate: 4/5
 * - TX Power: 14 dBm
 */

#ifndef LORA_H
#define LORA_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// CONSTANTS
// ============================================================================

#define LORA_FIFO_SIZE          256     // Maximum payload size
#define LORA_DEFAULT_TIMEOUT_MS 3000    // 3 second timeout

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

/**
 * Initialize LoRa module.
 * 
 * Configures:
 * - 915 MHz frequency
 * - Spreading Factor 10
 * - Bandwidth 62.5 kHz
 * - Coding Rate 4/5
 * - TX Power 14 dBm
 * 
 * Returns:
 *   true  - Initialization successful
 *   false - Initialization failed (check connections)
 */
bool lora_init(void);

/**
 * Send data via LoRa (blocking).
 * 
 * This function blocks until transmission is complete or timeout.
 * 
 * Parameters:
 *   data   - Pointer to data buffer
 *   length - Number of bytes to send (max LORA_FIFO_SIZE)
 * 
 * Returns:
 *   true  - Transmission successful
 *   false - Transmission failed
 * 
 * Example:
 *   char msg[] = "Hello World";
 *   lora_send(msg, strlen(msg));
 */
bool lora_send(const uint8_t* data, uint8_t length);

/**
 * Start receiving (non-blocking).
 * 
 * Puts LoRa into receive mode and returns immediately.
 * Check for received data with lora_receive_check().
 * 
 * Parameters:
 *   timeout_ms - Receive timeout in milliseconds (0 = continuous)
 * 
 * Example:
 *   lora_receive_start(5000);  // Listen for 5 seconds
 */
void lora_receive_start(uint32_t timeout_ms);

/**
 * Check if data has been received (non-blocking).
 * 
 * Parameters:
 *   buffer     - Buffer to store received data
 *   buffer_size - Size of buffer
 *   bytes_received - Output: number of bytes received
 *   rssi       - Output: signal strength (dBm)
 *   snr        - Output: signal-to-noise ratio (dB)
 * 
 * Returns:
 *   true  - Data received successfully
 *   false - No data yet (still waiting or timeout)
 * 
 * Example:
 *   uint8_t buf[LORA_FIFO_SIZE];
 *   uint8_t len;
 *   int16_t rssi;
 *   int8_t snr;
 *   
 *   lora_receive_start(5000);
 *   while (!lora_receive_check(buf, sizeof(buf), &len, &rssi, &snr)) {
 *       // Still waiting...
 *   }
 *   // Data received!
 */
bool lora_receive_check(uint8_t* buffer, uint8_t buffer_size, 
                        uint8_t* bytes_received, int16_t* rssi, int8_t* snr);

/**
 * Put LoRa module to sleep (low power mode).
 */
void lora_sleep(void);

/**
 * Wake LoRa module from sleep.
 */
void lora_wake(void);

/**
 * Read LoRa module version (for verification).
 * 
 * Returns:
 *   SX1276 version register value (should be 0x12)
 */
uint8_t lora_read_version(void);

#endif // LORA_H