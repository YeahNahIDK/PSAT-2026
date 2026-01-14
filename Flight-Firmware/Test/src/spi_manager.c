/*
 * ============================================================================
 * SPI MANAGER - MSP430FR2355 with UCB1
 * ============================================================================
 * 
 * SPI: eUSCI_B1 (UCB1)
 * Pins: P4.5 = CLK, P4.6 = MOSI, P4.7 = MISO
 * LoRa CS: P4.4
 * Clock: 250kHz SPI from 8MHz SMCLK
 */

#include "spi_manager.h"

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

static spi_mode_t current_mode = SPI_MODE_0;

// ============================================================================
// INITIALIZE SPI
// ============================================================================

void spi_init(void) {
    
    // ------------------------------------------------------------------------
    // PUT SPI IN RESET
    // ------------------------------------------------------------------------
    UCB1CTLW0 = UCSWRST;
    
    // ------------------------------------------------------------------------
    // CONFIGURE SPI PINS (P4.5, P4.6, P4.7)
    // ------------------------------------------------------------------------
    /*
     * - P4.5 = UCB1CLK  (SPI Clock)
     * - P4.6 = UCB1SIMO (MOSI - Master Out Slave In)
     * - P4.7 = UCB1SOMI (MISO - Master In Slave Out)
     */
    P4SEL0 |= BIT5 | BIT6 | BIT7;    // Set SEL0 = 1
    P4SEL1 &= ~(BIT5 | BIT6 | BIT7); // Set SEL1 = 0
    
    // ------------------------------------------------------------------------
    // CONFIGURE SPI CONTROL REGISTER
    // ------------------------------------------------------------------------
    /*
     * Settings:
     * - Master mode
     * - Synchronous (SPI)
     * - Mode 0 (CPOL=0, CPHA=0)
     * - MSB first
     * - 8-bit data
     * - SMCLK source
     */
    UCB1CTLW0 |= UCMST;           // Master mode
    UCB1CTLW0 |= UCSYNC;          // Synchronous (SPI) mode
    UCB1CTLW0 |= UCCKPL;          // Clock polarity for Mode 0
    UCB1CTLW0 |= UCMSB;           // MSB first
    UCB1CTLW0 |= UCSSEL__SMCLK;   // Use SMCLK as clock source
    
    // ------------------------------------------------------------------------
    // SET CLOCK SPEED
    // ------------------------------------------------------------------------
    /*
     * - SMCLK = 8MHz (DcoclkFreqSel::_8MHz, SmclkDiv::_1)
     * - Desired SPI = 250kHz
     * - Divider = 8,000,000 / 250,000 = 32
     */
    UCB1BRW = 32;  // 250kHz SPI clock
    
    // ------------------------------------------------------------------------
    // CONFIGURE CHIP SELECT PIN (P4.4)
    // ------------------------------------------------------------------------
    P4DIR |= BIT4;   // Make P4.4 output
    P4OUT |= BIT4;   // Set HIGH (deselected)
    
    // ------------------------------------------------------------------------
    // RELEASE SPI FROM RESET
    // ------------------------------------------------------------------------
    UCB1CTLW0 &= ~UCSWRST;
    
    current_mode = SPI_MODE_0;
}

// ============================================================================
// CHANGE SPI MODE
// ============================================================================

void spi_set_mode(spi_mode_t mode) {
    if (mode == current_mode) {
        return;
    }
    
    // Put SPI in reset
    UCB1CTLW0 |= UCSWRST;
    
    // Clear mode bits
    UCB1CTLW0 &= ~(UCCKPL | UCCKPH);
    
    // Set new mode
    switch (mode) {
        case SPI_MODE_0:  // CPOL=0, CPHA=0
            UCB1CTLW0 |= UCCKPL;
            break;
            
        case SPI_MODE_1:  // CPOL=0, CPHA=1
            UCB1CTLW0 |= UCCKPL;
            UCB1CTLW0 |= UCCKPH;
            break;
            
        case SPI_MODE_2:  // CPOL=1, CPHA=0
            // Both bits already cleared
            break;
            
        case SPI_MODE_3:  // CPOL=1, CPHA=1
            UCB1CTLW0 |= UCCKPH;
            break;
    }
    
    // Release from reset
    UCB1CTLW0 &= ~UCSWRST;
    
    current_mode = mode;
}

// ============================================================================
// SELECT DEVICE
// ============================================================================

void spi_select(volatile uint8_t* cs_port, uint8_t cs_pin, spi_mode_t mode) {
    spi_set_mode(mode);
    *cs_port &= ~cs_pin;  // Pull CS LOW
}

// ============================================================================
// DESELECT DEVICE
// ============================================================================

void spi_deselect(volatile uint8_t* cs_port, uint8_t cs_pin) {
    *cs_port |= cs_pin;  // Pull CS HIGH
}

// ============================================================================
// TRANSFER ONE BYTE
// ============================================================================

uint8_t spi_transfer(uint8_t tx_data) {
    // Wait for TX buffer ready
    while (!(UCB1IFG & UCTXIFG));
    
    // Write data to TX buffer
    UCB1TXBUF = tx_data;
    
    // Wait for RX buffer ready
    while (!(UCB1IFG & UCRXIFG));
    
    // Read and return received data
    return UCB1RXBUF;
}

// ============================================================================
// TRANSFER MULTIPLE BYTES
// ============================================================================

void spi_transfer_buffer(uint8_t* tx_buffer, uint8_t* rx_buffer, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        uint8_t tx_byte = tx_buffer ? tx_buffer[i] : 0x00;
        uint8_t rx_byte = spi_transfer(tx_byte);
        
        if (rx_buffer) {
            rx_buffer[i] = rx_byte;
        }
    }
}

/*
 * ============================================================================
 * CONFIGURATION SUMMARY
 * ============================================================================
 * 
 * Chip: MSP430FR2355
 * SPI Module: eUSCI_B1 (UCB1)
 * 
 * Pins:
 * - P4.5: UCB1CLK  (SPI Clock)
 * - P4.6: UCB1SIMO (MOSI)
 * - P4.7: UCB1SOMI (MISO)
 * - P4.4: LoRa CS (GPIO)
 * 
 * Clock:
 * - SMCLK: 8MHz
 * - SPI: 250kHz (divider = 32)
 * 
 * Mode: SPI Mode 0 (CPOL=0, CPHA=0)
 * 
 * This matches your Rust configuration exactly!
 */