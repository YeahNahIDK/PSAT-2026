/*
 * ============================================================================
 * SPI MANAGER - Header File
 * ============================================================================
 * 
 * MSP430FR2355 using eUSCI_B1
 * Pins: P4.5=CLK, P4.6=MOSI, P4.7=MISO, P4.4=LoRa CS
 */

#ifndef SPI_MANAGER_H
#define SPI_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <msp430.h>

// ============================================================================
// SPI MODE DEFINITIONS
// ============================================================================

typedef enum {
    SPI_MODE_0 = 0,  // CPOL=0, CPHA=0 (LoRa uses this)
    SPI_MODE_1 = 1,  // CPOL=0, CPHA=1
    SPI_MODE_2 = 2,  // CPOL=1, CPHA=0
    SPI_MODE_3 = 3   // CPOL=1, CPHA=1
} spi_mode_t;

// ============================================================================
// CHIP SELECT PIN DEFINITIONS
// ============================================================================

/*
 * LoRa CS is on P4.4
 */
#define LORA_CS_PORT    P4OUT
#define LORA_CS_DIR     P4DIR
#define LORA_CS_PIN     BIT4

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

/**
 * Initialize SPI peripheral (eUSCI_B1).
 * Configures:
 * - P4.5, P4.6, P4.7 as SPI pins
 * - 250kHz SPI clock from 8MHz SMCLK
 * - Mode 0 (CPOL=0, CPHA=0)
 * - Master mode, MSB first
 */
void spi_init(void);

/**
 * Change SPI mode.
 * Only changes if different from current mode to avoid overhead.
 * 
 * @param mode - SPI mode (0-3)
 */
void spi_set_mode(spi_mode_t mode);

/**
 * Select SPI device (pull CS low).
 * Also sets correct SPI mode for the device.
 * 
 * @param cs_port - Pointer to CS port (e.g., &P4OUT)
 * @param cs_pin  - CS pin bit (e.g., BIT4)
 * @param mode    - Required SPI mode for this device
 * 
 * Example:
 *   spi_select(&LORA_CS_PORT, LORA_CS_PIN, SPI_MODE_0);
 */
void spi_select(volatile uint8_t* cs_port, uint8_t cs_pin, spi_mode_t mode);

/**
 * Deselect SPI device (pull CS high).
 * 
 * @param cs_port - Pointer to CS port
 * @param cs_pin  - CS pin bit
 * 
 * Example:
 *   spi_deselect(&LORA_CS_PORT, LORA_CS_PIN);
 */
void spi_deselect(volatile uint8_t* cs_port, uint8_t cs_pin);

/**
 * Transfer one byte via SPI (send and receive simultaneously).
 * 
 * @param tx_data - Byte to transmit
 * @return Byte received from device
 * 
 * Example:
 *   uint8_t response = spi_transfer(0x42);
 */
uint8_t spi_transfer(uint8_t tx_data);

/**
 * Transfer multiple bytes via SPI.
 * 
 * @param tx_buffer - Data to transmit (or NULL if only receiving)
 * @param rx_buffer - Buffer for received data (or NULL if only transmitting)
 * @param length    - Number of bytes to transfer
 * 
 * Examples:
 *   // Send only:
 *   spi_transfer_buffer(data, NULL, 5);
 *   
 *   // Receive only:
 *   spi_transfer_buffer(NULL, data, 5);
 *   
 *   // Both:
 *   spi_transfer_buffer(tx_data, rx_data, 5);
 */
void spi_transfer_buffer(uint8_t* tx_buffer, uint8_t* rx_buffer, uint16_t length);

#endif // SPI_MANAGER_H