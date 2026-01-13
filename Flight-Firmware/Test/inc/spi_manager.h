/*
 * Manages SPI communication for multiple devices.
 * Handles mode switching and chip select management.
 */

#ifndef SPI_MANAGER_H
#define SPI_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <msp430.h>

// ============================================================================
// SPI MODE DEFINITIONS
// ============================================================================

/*
 * SPI has 4 modes based on Clock Polarity (CPOL) and Clock Phase (CPHA):
 *
 * Mode 0: CPOL=0, CPHA=0
 * Mode 1: CPOL=0, CPHA=1
 * Mode 2: CPOL=1, CPHA=0
 * Mode 3: CPOL=1, CPHA=1
 */

typedef enum {
    SPI_MODE_0 = 0,
    SPI_MODE_1 = 1,
    SPI_MODE_2 = 2,
    SPI_MODE_3 = 3
} spi_mode_t;

// ============================================================================
// CHIP SELECT PIN DEFINITIONS
// ============================================================================

// LoRa CS pin
#define LORA_CS_PORT    P2OUT
#define LORA_CS_DIR     P2DIR
#define LORA_CS_PIN     BIT0

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

/*
 * Initialize SPI peripheral.
 */
void spi_init(void);

/*
 * Change SPI mode (clock polarity/phase).
 * Only changes if different from current mode.
 * 
 * Parameters:
 *   mode - SPI mode (0-3)
 */
void spi_set_mode(spi_mode_t mode);

/*
 * Select a device (pull CS low).
 * Also sets the correct SPI mode for that device.
 * 
 * Parameters:
 *   cs_port - Pointer to CS port register (e.g., &P2OUT)
 *   cs_pin  - CS pin bit (e.g., BIT0)
 *   mode    - Required SPI mode for this device
 */
void spi_select(volatile uint8_t* cs_port, uint8_t cs_pin, spi_mode_t mode);

/*
 * Deselect a device (pull CS high).
 * 
 * Parameters:
 *   cs_port - Pointer to CS port register
 *   cs_pin  - CS pin bit
 */
void spi_deselect(volatile uint8_t* cs_port, uint8_t cs_pin);

/*
 * Transfer one byte via SPI (send and receive simultaneously).
 * 
 * Parameters:
 *   tx_data - Byte to transmit
 * 
 * Returns:
 *   Byte received from device
 */
uint8_t spi_transfer(uint8_t tx_data);

/*
 * Transfer multiple bytes via SPI.
 * 
 * Parameters:
 *   tx_buffer - Data to transmit (or NULL if only receiving)
 *   rx_buffer - Buffer for received data (or NULL if only transmitting)
 *   length    - Number of bytes to transfer
 */
void spi_transfer_buffer(uint8_t* tx_buffer, uint8_t* rx_buffer, uint16_t length);

#endif // SPI_MANAGER_H