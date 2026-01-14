/*
 * ============================================================================
 * LORA DRIVER - Implementation
 * ============================================================================
 * 
 * This is a simplified driver focused on basic send/receive.
 * Based on Semtech SX1276 datasheet.
 */

#include "lora.h"
#include "spi_manager.h"
#include <string.h>

// ============================================================================
// SX1276 REGISTER ADDRESSES
// ============================================================================

/*
 * The SX1276 has many registers. Here are the essential ones.
 * Full register map is in the SX1276 datasheet.
 */

// Common registers
#define REG_FIFO                    0x00
#define REG_OP_MODE                 0x01
#define REG_FRF_MSB                 0x06
#define REG_FRF_MID                 0x07
#define REG_FRF_LSB                 0x08
#define REG_PA_CONFIG               0x09
#define REG_LNA                     0x0C
#define REG_FIFO_ADDR_PTR           0x0D
#define REG_FIFO_TX_BASE_ADDR       0x0E
#define REG_FIFO_RX_BASE_ADDR       0x0F
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_IRQ_FLAGS               0x12
#define REG_RX_NB_BYTES             0x13
#define REG_PKT_RSSI_VALUE          0x1A
#define REG_PKT_SNR_VALUE           0x1B
#define REG_MODEM_CONFIG_1          0x1D
#define REG_MODEM_CONFIG_2          0x1E
#define REG_PREAMBLE_MSB            0x20
#define REG_PREAMBLE_LSB            0x21
#define REG_PAYLOAD_LENGTH          0x22
#define REG_MODEM_CONFIG_3          0x26
#define REG_FREQ_ERROR_MSB          0x28
#define REG_FREQ_ERROR_MID          0x29
#define REG_FREQ_ERROR_LSB          0x2A
#define REG_RSSI_WIDEBAND           0x2C
#define REG_DETECTION_OPTIMIZE      0x31
#define REG_INVERTIQ                0x33
#define REG_DETECTION_THRESHOLD     0x37
#define REG_SYNC_WORD               0x39
#define REG_DIO_MAPPING_1           0x40
#define REG_VERSION                 0x42
#define REG_PA_DAC                  0x4D

// ============================================================================
// OPERATING MODES
// ============================================================================

#define MODE_LONG_RANGE_MODE        0x80  // LoRa mode (vs FSK)
#define MODE_SLEEP                  0x00
#define MODE_STDBY                  0x01
#define MODE_TX                     0x03
#define MODE_RX_CONTINUOUS          0x05
#define MODE_RX_SINGLE              0x06

// ============================================================================
// IRQ FLAGS
// ============================================================================

#define IRQ_TX_DONE_MASK            0x08
#define IRQ_RX_DONE_MASK            0x40
#define IRQ_PAYLOAD_CRC_ERROR_MASK  0x20

// ============================================================================
// PA CONFIG
// ============================================================================

#define PA_BOOST                    0x80

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

/**
 * Write to a register.
 * 
 * SX1276 write: set bit 7 of address to 1 (0x80 | address)
 */
static void lora_write_register(uint8_t address, uint8_t value) {
    spi_select(&LORA_CS_PORT, LORA_CS_PIN, SPI_MODE_0);
    spi_transfer(0x80 | address);  // Write command
    spi_transfer(value);
    spi_deselect(&LORA_CS_PORT, LORA_CS_PIN);
}

/**
 * Read from a register.
 * 
 * SX1276 read: bit 7 of address = 0 (just use address as-is)
 */
static uint8_t lora_read_register(uint8_t address) {
    uint8_t value;
    
    spi_select(&LORA_CS_PORT, LORA_CS_PIN, SPI_MODE_0);
    spi_transfer(address & 0x7F);  // Read command (bit 7 = 0)
    value = spi_transfer(0x00);     // Dummy byte to read
    spi_deselect(&LORA_CS_PORT, LORA_CS_PIN);
    
    return value;
}

/**
 * Set operating mode.
 */
static void lora_set_mode(uint8_t mode) {
    lora_write_register(REG_OP_MODE, MODE_LONG_RANGE_MODE | mode);
}

/**
 * Wait for a specific IRQ flag.
 * 
 * Returns true if flag set, false if timeout.
 */
static bool lora_wait_for_irq(uint8_t irq_mask, uint32_t timeout_ms) {
    // TODO: Use actual timer here instead of loop counter
    // For now, approximate with loop iterations
    uint32_t count = timeout_ms * 1000;  // Rough approximation
    
    while (count--) {
        uint8_t irq_flags = lora_read_register(REG_IRQ_FLAGS);
        if (irq_flags & irq_mask) {
            // Clear the flag
            lora_write_register(REG_IRQ_FLAGS, irq_mask);
            return true;
        }
    }
    
    return false;  // Timeout
}

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

bool lora_init(void) {
    /*
     * INITIALIZATION SEQUENCE:
     * 1. Put in sleep mode
     * 2. Set LoRa mode
     * 3. Set frequency (915 MHz)
     * 4. Set spreading factor, bandwidth, coding rate
     * 5. Set TX power
     * 6. Set preamble length
     * 7. Go to standby mode
     */
    
    // Put in sleep mode first
    lora_set_mode(MODE_SLEEP);
    
    // Small delay for mode change
    // TODO: Use actual delay function
    for (volatile uint32_t i = 0; i < 10000; i++);
    
    // Verify we can communicate - read version register
    uint8_t version = lora_read_version();
    if (version != 0x12) {
        // Wrong version or no response
        return false;
    }
    
    // Set LoRa mode and standby
    lora_set_mode(MODE_STDBY);
    
    /*
     * SET FREQUENCY: 915 MHz
     * 
     * Formula: Frf = (Freq * 2^19) / 32MHz
     * For 915 MHz: Frf = 0xE4C000
     */
    lora_write_register(REG_FRF_MSB, 0xE4);
    lora_write_register(REG_FRF_MID, 0xC0);
    lora_write_register(REG_FRF_LSB, 0x00);
    
    /*
     * SET MODEM CONFIG 1:
     * - Bandwidth: 62.5 kHz (0b0110 << 4)
     * - Coding Rate: 4/5 (0b001 << 1)
     * - Implicit Header: OFF (0b0)
     * 
     * Bandwidth values:
     * 0000 = 7.8 kHz
     * 0001 = 10.4 kHz
     * 0010 = 15.6 kHz
     * 0011 = 20.8 kHz
     * 0100 = 31.25 kHz
     * 0101 = 41.7 kHz
     * 0110 = 62.5 kHz  ← We use this
     * 0111 = 125 kHz
     * 1000 = 250 kHz
     * 1001 = 500 kHz
     */
    lora_write_register(REG_MODEM_CONFIG_1, 0x62);  // BW=62.5kHz, CR=4/5, Explicit
    
    /*
     * SET MODEM CONFIG 2:
     * - Spreading Factor: 10 (0b1010 << 4)
     * - TX Continuous: OFF
     * - CRC: ON (bit 2)
     */
    lora_write_register(REG_MODEM_CONFIG_2, 0xA4);  // SF10, CRC ON
    
    /*
     * SET MODEM CONFIG 3:
     * - Low Data Rate Optimize: ON (required for SF10 with BW62.5)
     * - AGC Auto: ON
     */
    lora_write_register(REG_MODEM_CONFIG_3, 0x0C);
    
    /*
     * SET TX POWER: 14 dBm
     * Using PA_BOOST pin (required for >14dBm)
     * 
     * Power = 17 - (15 - OutputPower)
     * For 14dBm: OutputPower = 12
     */
    lora_write_register(REG_PA_CONFIG, PA_BOOST | 12);
    
    // Set preamble length to 8
    lora_write_register(REG_PREAMBLE_MSB, 0x00);
    lora_write_register(REG_PREAMBLE_LSB, 0x08);
    
    // Set FIFO base addresses
    lora_write_register(REG_FIFO_TX_BASE_ADDR, 0x00);
    lora_write_register(REG_FIFO_RX_BASE_ADDR, 0x00);
    
    // Set sync word (0x12 = private network)
    lora_write_register(REG_SYNC_WORD, 0x12);
    
    // Set LNA gain
    lora_write_register(REG_LNA, 0x23);  // Max gain
    
    return true;
}

bool lora_send(const uint8_t* data, uint8_t length) {
    /*
     * TRANSMIT SEQUENCE:
     * 1. Go to standby mode
     * 2. Clear IRQ flags
     * 3. Set FIFO pointer to TX base
     * 4. Write payload length
     * 5. Write data to FIFO
     * 6. Set TX mode
     * 7. Wait for TX done IRQ
     */
    
    // Check length
    if (length > LORA_FIFO_SIZE) {
        return false;
    }
    
    // Go to standby
    lora_set_mode(MODE_STDBY);
    
    // Clear IRQ flags
    lora_write_register(REG_IRQ_FLAGS, 0xFF);
    
    // Set FIFO pointer to TX base
    lora_write_register(REG_FIFO_ADDR_PTR, 0x00);
    
    // Write payload length
    lora_write_register(REG_PAYLOAD_LENGTH, length);
    
    // Write data to FIFO
    spi_select(&LORA_CS_PORT, LORA_CS_PIN, SPI_MODE_0);
    spi_transfer(0x80 | REG_FIFO);  // Write to FIFO
    for (uint8_t i = 0; i < length; i++) {
        spi_transfer(data[i]);
    }
    spi_deselect(&LORA_CS_PORT, LORA_CS_PIN);
    
    // Start transmission
    lora_set_mode(MODE_TX);
    
    // Wait for TX done
    bool success = lora_wait_for_irq(IRQ_TX_DONE_MASK, LORA_DEFAULT_TIMEOUT_MS);
    
    // Go back to standby
    lora_set_mode(MODE_STDBY);
    
    return success;
}

void lora_receive_start(uint32_t timeout_ms) {
    /*
     * START RECEIVE:
     * 1. Go to standby
     * 2. Clear IRQ flags
     * 3. Set FIFO pointer to RX base
     * 4. Set RX mode (continuous or single)
     */
    
    // Go to standby
    lora_set_mode(MODE_STDBY);
    
    // Clear IRQ flags
    lora_write_register(REG_IRQ_FLAGS, 0xFF);
    
    // Set FIFO pointer to RX base
    lora_write_register(REG_FIFO_ADDR_PTR, 0x00);
    
    // Start continuous RX
    // TODO: Implement timeout using timer
    lora_set_mode(MODE_RX_CONTINUOUS);
}

bool lora_receive_check(uint8_t* buffer, uint8_t buffer_size, 
                        uint8_t* bytes_received, int16_t* rssi, int8_t* snr) {
    /*
     * CHECK FOR RECEIVED DATA:
     * 1. Check RX done IRQ
     * 2. If set, read payload length
     * 3. Read data from FIFO
     * 4. Read RSSI and SNR
     * 5. Clear IRQ flag
     */
    
    // Check RX done flag
    uint8_t irq_flags = lora_read_register(REG_IRQ_FLAGS);
    
    if (!(irq_flags & IRQ_RX_DONE_MASK)) {
        // No data yet
        return false;
    }
    
    // Check for CRC error
    if (irq_flags & IRQ_PAYLOAD_CRC_ERROR_MASK) {
        // CRC error - clear flags and return false
        lora_write_register(REG_IRQ_FLAGS, 0xFF);
        return false;
    }
    
    // Read payload length
    uint8_t length = lora_read_register(REG_RX_NB_BYTES);
    *bytes_received = length;
    
    // Check buffer size
    if (length > buffer_size) {
        length = buffer_size;  // Truncate
    }
    
    // Read current RX address
    uint8_t rx_addr = lora_read_register(REG_FIFO_RX_CURRENT_ADDR);
    lora_write_register(REG_FIFO_ADDR_PTR, rx_addr);
    
    // Read data from FIFO
    spi_select(&LORA_CS_PORT, LORA_CS_PIN, SPI_MODE_0);
    spi_transfer(REG_FIFO);  // Read from FIFO
    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = spi_transfer(0x00);
    }
    spi_deselect(&LORA_CS_PORT, LORA_CS_PIN);
    
    // Read RSSI
    int16_t rssi_raw = lora_read_register(REG_PKT_RSSI_VALUE);
    *rssi = -164 + rssi_raw;  // For 915MHz band
    
    // Read SNR
    int8_t snr_raw = lora_read_register(REG_PKT_SNR_VALUE);
    *snr = snr_raw / 4;  // SNR is in 0.25 dB steps
    
    // Clear IRQ flags
    lora_write_register(REG_IRQ_FLAGS, 0xFF);
    
    return true;
}

void lora_sleep(void) {
    lora_set_mode(MODE_SLEEP);
}

void lora_wake(void) {
    lora_set_mode(MODE_STDBY);
}

uint8_t lora_read_version(void) {
    return lora_read_register(REG_VERSION);
}

/*
 * ============================================================================
 * USAGE EXAMPLES
 * ============================================================================
 * 
 * Example 1: Send a message
 * 
 *   char msg[] = "Hello World";
 *   if (lora_send((uint8_t*)msg, strlen(msg))) {
 *       // Success
 *   } else {
 *       // Failed
 *   }
 * 
 * 
 * Example 2: Receive a message
 * 
 *   uint8_t buffer[LORA_FIFO_SIZE];
 *   uint8_t len;
 *   int16_t rssi;
 *   int8_t snr;
 *   
 *   lora_receive_start(0);  // Continuous RX
 *   
 *   while (1) {
 *       if (lora_receive_check(buffer, sizeof(buffer), &len, &rssi, &snr)) {
 *           // Data received!
 *           // buffer contains 'len' bytes
 *           // rssi = signal strength
 *           // snr = signal quality
 *           
 *           // Process data...
 *           
 *           // Start listening again
 *           lora_receive_start(0);
 *       }
 *   }
 */