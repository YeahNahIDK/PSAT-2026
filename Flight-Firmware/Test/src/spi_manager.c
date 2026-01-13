/*
 * ============================================================================
 * SPI MANAGER - Implementation
 * ============================================================================
 * 
 * Implements SPI communication for MSP430.
 * 
 * IMPORTANT: This code is written for MSP430FR2355 using eUSCI_B0.
 */

#include "spi_manager.h"

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

// Track current SPI mode to avoid unnecessary reconfiguration
static spi_mode_t current_mode = SPI_MODE_0;

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

void spi_init(void) {
    /*
     * INITIALIZE SPI PERIPHERAL
     * 
     * Configuration:
     * - Master mode
     * - 8-bit data
     * - MSB first
     * - Mode 0 initially (CPOL=0, CPHA=0)
     * - Clock from SMCLK
     * 
     * Pin configuration (MSP430FR2355 example):
     * - P1.4 = UCB0CLK  (SPI Clock)
     * - P1.6 = UCB0SIMO (MOSI - Master Out Slave In)
     * - P1.7 = UCB0SOMI (MISO - Master In Slave Out)
     * 
     * ADJUST THESE PINS FOR YOUR DEVICE!
     */
    
    // Put eUSCI in reset state while configuring
    UCB0CTLW0 = UCSWRST;
    
    // Configure SPI pins
    // Check your device datasheet for correct pin mapping
    P1SEL0 |= BIT4 | BIT6 | BIT7;  // Set pins to SPI function
    P1SEL1 &= ~(BIT4 | BIT6 | BIT7);
    
    /*
     * CONFIGURE SPI CONTROL REGISTER (UCB0CTLW0):
     * 
     * UCSWRST - Software reset enabled (until we finish configuring)
     * UCMST   - Master mode
     * UCSYNC  - Synchronous mode (SPI)
     * UCCKPL  - Clock polarity (1 = inactive state is high for Mode 0)
     * UCMSB   - MSB first
     * UC7BIT  - 8-bit data (0 = 8-bit, 1 = 7-bit)
     * UCSSEL  - Clock source selection
     */
    UCB0CTLW0 |= UCMST;      // Master mode
    UCB0CTLW0 |= UCSYNC;     // Synchronous (SPI) mode
    UCB0CTLW0 |= UCCKPL;     // Clock polarity for Mode 0
    UCB0CTLW0 |= UCMSB;      // MSB first
    UCB0CTLW0 |= UCSSEL__SMCLK;  // Use SMCLK
    
    /*
     * SET CLOCK DIVIDER:
     * SPI Clock = SMCLK / UCBRx
     * 
     * Example with 1MHz SMCLK:
     * - UCBRx = 2  → 500kHz SPI clock
     * - UCBRx = 10 → 100kHz SPI clock
     * 
     * Start conservative (slower). Check device datasheets for max speed.
     */
    UCB0BRW = 10;  // Divide SMCLK by 10 (100kHz if SMCLK=1MHz)
    
    // Configure all CS pins as outputs and set HIGH (deselected)
    LORA_CS_DIR |= LORA_CS_PIN;   // Set as output
    LORA_CS_PORT |= LORA_CS_PIN;  // Set HIGH (deselected)
    
    // Add more CS pins here as needed
    
    // Release eUSCI from reset
    UCB0CTLW0 &= ~UCSWRST;
    
    // Set initial mode
    current_mode = SPI_MODE_0;
}

void spi_set_mode(spi_mode_t mode) {
    /*
     * CHANGE SPI MODE
     * 
     * Only reconfigure if mode is different from current.
     * Must put SPI in reset to change settings.
     * 
     * Mode settings:
     * Mode 0: CPOL=0, CPHA=0 → UCCKPL=1, UCCKPH=0
     * Mode 1: CPOL=0, CPHA=1 → UCCKPL=1, UCCKPH=1
     * Mode 2: CPOL=1, CPHA=0 → UCCKPL=0, UCCKPH=0
     * Mode 3: CPOL=1, CPHA=1 → UCCKPL=0, UCCKPH=1
     * 
     * Note: MSP430 register bits are inverted from standard CPOL definition
     */
    
    if (mode == current_mode) {
        return;  // Already in correct mode
    }
    
    // Put SPI in reset
    UCB0CTLW0 |= UCSWRST;
    
    // Clear mode bits
    UCB0CTLW0 &= ~(UCCKPL | UCCKPH);
    
    // Set new mode
    switch (mode) {
        case SPI_MODE_0:
            UCB0CTLW0 |= UCCKPL;   // CPOL=0 (inverted)
            // UCCKPH already 0
            break;
            
        case SPI_MODE_1:
            UCB0CTLW0 |= UCCKPL;   // CPOL=0
            UCB0CTLW0 |= UCCKPH;   // CPHA=1
            break;
            
        case SPI_MODE_2:
            // UCCKPL already 0      // CPOL=1
            // UCCKPH already 0      // CPHA=0
            break;
            
        case SPI_MODE_3:
            // UCCKPL already 0      // CPOL=1
            UCB0CTLW0 |= UCCKPH;   // CPHA=1
            break;
    }
    
    // Release from reset
    UCB0CTLW0 &= ~UCSWRST;
    
    // Update current mode
    current_mode = mode;
}

void spi_select(volatile uint8_t* cs_port, uint8_t cs_pin, spi_mode_t mode) {
    /*
     * SELECT SPI DEVICE
     * 
     * 1. Set correct SPI mode for this device
     * 2. Pull CS pin LOW (active low)
     */
    
    // Ensure correct mode
    spi_set_mode(mode);
    
    // Pull CS low to select device
    *cs_port &= ~cs_pin;
}

void spi_deselect(volatile uint8_t* cs_port, uint8_t cs_pin) {
    /*
     * DESELECT SPI DEVICE
     * 
     * Pull CS pin HIGH (inactive)
     */
    
    *cs_port |= cs_pin;
}

uint8_t spi_transfer(uint8_t tx_data) {
    /*
     * TRANSFER ONE BYTE
     * 
     * SPI is full-duplex: always sends and receives simultaneously.
     * 
     * Process:
     * 1. Wait for TX buffer to be empty
     * 2. Write byte to TX buffer
     * 3. Wait for RX buffer to have data
     * 4. Read and return received byte
     * 
     * Even if you don't care about received data, you MUST read it
     * to clear the buffer for the next transfer.
     */
    
    // Wait for TX buffer to be ready
    while (!(UCB0IFG & UCTXIFG));
    
    // Write data to TX buffer
    UCB0TXBUF = tx_data;
    
    // Wait for RX buffer to have data
    while (!(UCB0IFG & UCRXIFG));
    
    // Read and return received data
    return UCB0RXBUF;
}

void spi_transfer_buffer(uint8_t* tx_buffer, uint8_t* rx_buffer, uint16_t length) {
    /*
     * TRANSFER MULTIPLE BYTES
     * 
     * Parameters:
     * - tx_buffer: Data to send (or NULL if only receiving)
     * - rx_buffer: Buffer for received data (or NULL if only transmitting)
     * - length: Number of bytes to transfer
     * 
     * Example uses:
     * - Send only:    spi_transfer_buffer(data, NULL, 10);
     * - Receive only: spi_transfer_buffer(NULL, data, 10);
     * - Both:         spi_transfer_buffer(tx_data, rx_data, 10);
     */
    
    for (uint16_t i = 0; i < length; i++) {
        // Get byte to transmit (or 0x00 if tx_buffer is NULL)
        uint8_t tx_byte = tx_buffer ? tx_buffer[i] : 0x00;
        
        // Transfer byte
        uint8_t rx_byte = spi_transfer(tx_byte);
        
        // Store received byte if rx_buffer provided
        if (rx_buffer) {
            rx_buffer[i] = rx_byte;
        }
    }
}

/*
 * ============================================================================
 * USAGE EXAMPLES
 * ============================================================================
 * 
 * Example 1: Read a register from LoRa
 * 
 *   uint8_t reg_address = 0x42;
 *   uint8_t reg_value;
 *   
 *   spi_select(&LORA_CS_PORT, LORA_CS_PIN, SPI_MODE_0);
 *   spi_transfer(reg_address);  // Send register address
 *   reg_value = spi_transfer(0x00);  // Read value
 *   spi_deselect(&LORA_CS_PORT, LORA_CS_PIN);
 * 
 * 
 * Example 2: Write to a register
 * 
 *   spi_select(&LORA_CS_PORT, LORA_CS_PIN, SPI_MODE_0);
 *   spi_transfer(0x80 | reg_address);  // Write command (bit 7 = write)
 *   spi_transfer(value);  // Send value
 *   spi_deselect(&LORA_CS_PORT, LORA_CS_PIN);
 * 
 * 
 * Example 3: Transfer a buffer
 * 
 *   uint8_t tx_data[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
 *   uint8_t rx_data[5];
 *   
 *   spi_select(&LORA_CS_PORT, LORA_CS_PIN, SPI_MODE_0);
 *   spi_transfer_buffer(tx_data, rx_data, 5);
 *   spi_deselect(&LORA_CS_PORT, LORA_CS_PIN);
 */