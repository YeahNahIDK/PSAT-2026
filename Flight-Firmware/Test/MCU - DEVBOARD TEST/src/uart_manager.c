/*
 * ============================================================================
 * UART MANAGER - Implementation
 * ============================================================================
 * 
 * Implements UART communication for MSP430FR2355
 * Supports UCA0 and UCA1
 */

#include "./inc/uart_manager.h"
#include <msp430.h>
#include <string.h>

// ============================================================================
// CIRCULAR BUFFER STRUCTURE
// ============================================================================

typedef struct {
    uint8_t buffer[UART_RX_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} circular_buffer_t;

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

// RX buffers for each UART module
static circular_buffer_t rx_buffers[UART_COUNT] = {0};

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

static uint16_t buffer_available(circular_buffer_t* buf) {
    /*
     * Calculate number of bytes available in buffer
     * 
     * Two cases:
     * 1. Normal: head >= tail
     *    Available = head - tail
     * 
     * 2. Wrapped: head < tail (buffer has wrapped around)
     *    Available = (space from tail to end) + (space from start to head)
     *              = (BUFFER_SIZE - tail) + head
     */
    if (buf->head >= buf->tail) {
        return buf->head - buf->tail;
    } else {
        return (UART_RX_BUFFER_SIZE - buf->tail) + buf->head;
    }
}

static bool buffer_read(circular_buffer_t* buf, uint8_t* data) {
    /*
     * Read one byte from circular buffer
     */
    if (buf->head == buf->tail) {
        return false;  // Buffer empty
    }
    
    *data = buf->buffer[buf->tail];
    buf->tail = (buf->tail + 1) % UART_RX_BUFFER_SIZE;
    return true;
}

static bool buffer_peek(circular_buffer_t* buf, uint8_t* data) {
    /*
     * Peek at next byte without removing it
     */
    if (buf->head == buf->tail) {
        return false;  // Buffer empty
    }
    
    *data = buf->buffer[buf->tail];
    return true;
}

static void buffer_write(circular_buffer_t* buf, uint8_t data) {
    /*
     * Write one byte to circular buffer (called from ISR)
     */
    uint16_t next_head = (buf->head + 1) % UART_RX_BUFFER_SIZE;
    
    // Only store if buffer not full
    if (next_head != buf->tail) {
        buf->buffer[buf->head] = data;
        buf->head = next_head;
    }
    // If full, oldest data is lost (overwritten)
}

static void buffer_clear(circular_buffer_t* buf) {
    /*
     * Clear buffer
     */
    buf->head = 0;
    buf->tail = 0;
}

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

void uart_init(uart_module_t module, uint32_t baud_rate) {
    /*
     * INITIALIZE UART MODULE
     * 
     * Baud rate calculation for 8MHz SMCLK:
     * N = SMCLK / baud_rate
     * 
     * For oversampling mode (recommended):
     * UCBRx = INT(N/16)
     * UCBRFx = INT(((N/16) - INT(N/16)) * 16)
     * UCBRSx = from table in datasheet
     */
    
    if (module >= UART_COUNT) {
        return;
    }
    
    // Clear buffer
    buffer_clear(&rx_buffers[module]);
    
    // Calculate baud rate settings
    uint32_t n = 8000000UL / baud_rate;  // Assuming 8MHz SMCLK
    uint16_t ucbrx = n / 16;
    uint16_t ucbrfx = ((n % 16) * 16) / 16;
    uint16_t ucbrsx = 0;
    
    // UCBRSx values from datasheet (approximations)
    // These are for common baud rates with 8MHz SMCLK
    switch (baud_rate) {
        case 9600:   ucbrsx = 0x49; break;
        case 19200:  ucbrsx = 0x4A; break;
        case 38400:  ucbrsx = 0x55; break;
        case 57600:  ucbrsx = 0x55; break;
        case 115200: ucbrsx = 0x49; break;
        default:     ucbrsx = 0x00; break;
    }
    
    if (module == UART_A0) {
        // ============================================================
        // UCA0 INITIALIZATION (LoRa on MSP430FR2433)
        // ============================================================
        
        // Hold UART in reset
        UCA0CTLW0 = UCSWRST;
        
        // Configure pins: P1.4=TX, P1.5=RX (default for UCA0 on FR2433)
        P1SEL0 |= BIT4 | BIT5;
        P1SEL1 &= ~(BIT4 | BIT5);
        
        // Configure UART
        UCA0CTLW0 |= UCSSEL__SMCLK;
        UCA0BRW = ucbrx;
        UCA0MCTLW = (ucbrsx << 8) | (ucbrfx << 4) | UCOS16;
        
        // Release from reset
        UCA0CTLW0 &= ~UCSWRST;
        
        // Enable RX interrupt
        UCA0IE |= UCRXIE;
    }
    else if (module == UART_A1) {
        // ============================================================
        // UCA1 INITIALIZATION (GPS on MSP430FR2433)
        // ============================================================
        
        // Hold UART in reset
        UCA1CTLW0 = UCSWRST;
        
        // Configure pins: P1.6=TX, P1.7=RX (for UCA1 on FR2433)
        P1SEL0 |= BIT6 | BIT7;
        P1SEL1 &= ~(BIT6 | BIT7);
        
        // Configure UART
        UCA1CTLW0 |= UCSSEL__SMCLK;
        UCA1BRW = ucbrx;
        UCA1MCTLW = (ucbrsx << 8) | (ucbrfx << 4) | UCOS16;
        
        // Release from reset
        UCA1CTLW0 &= ~UCSWRST;
        
        // Enable RX interrupt
        UCA1IE |= UCRXIE;
    }
}

void uart_write_byte(uart_module_t module, uint8_t data) {
    if (module == UART_A0) {
        while (!(UCA0IFG & UCTXIFG));  // Wait for TX ready
        UCA0TXBUF = data;
    }
    else if (module == UART_A1) {
        while (!(UCA1IFG & UCTXIFG));
        UCA1TXBUF = data;
    }
}

void uart_write_string(uart_module_t module, const char* str) {
    if (str == NULL) return;
    
    while (*str) {
        uart_write_byte(module, *str++);
    }
}

void uart_write_buffer(uart_module_t module, const uint8_t* buffer, uint16_t length) {
    if (buffer == NULL) return;
    
    for (uint16_t i = 0; i < length; i++) {
        uart_write_byte(module, buffer[i]);
    }
}

uint16_t uart_available(uart_module_t module) {
    if (module >= UART_COUNT) {
        return 0;
    }
    
    return buffer_available(&rx_buffers[module]);
}

bool uart_read_byte(uart_module_t module, uint8_t* data) {
    if (module >= UART_COUNT || data == NULL) {
        return false;
    }
    
    return buffer_read(&rx_buffers[module], data);
}

uint16_t uart_read_buffer(uart_module_t module, uint8_t* buffer, uint16_t max_length) {
    if (module >= UART_COUNT || buffer == NULL) {
        return 0;
    }
    
    uint16_t count = 0;
    while (count < max_length && buffer_read(&rx_buffers[module], &buffer[count])) {
        count++;
    }
    
    return count;
}

bool uart_peek(uart_module_t module, uint8_t* data) {
    if (module >= UART_COUNT || data == NULL) {
        return false;
    }
    
    return buffer_peek(&rx_buffers[module], data);
}

void uart_flush(uart_module_t module) {
    if (module >= UART_COUNT) {
        return;
    }
    
    buffer_clear(&rx_buffers[module]);
}

uint16_t uart_read_until(uart_module_t module, uint8_t* buffer, uint16_t max_length, uint8_t terminator) {
    if (module >= UART_COUNT || buffer == NULL) {
        return 0;
    }
    
    uint16_t count = 0;
    uint8_t byte;
    
    while (count < max_length && buffer_read(&rx_buffers[module], &byte)) {
        buffer[count++] = byte;
        
        if (byte == terminator) {
            break;  // Found terminator
        }
    }
    
    return count;
}

// ============================================================================
// INTERRUPT SERVICE ROUTINES
// ============================================================================

/*
 * UCA0 INTERRUPT (Debug/General Purpose UART)
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG)) {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
            buffer_write(&rx_buffers[UART_A0], UCA0RXBUF);
            break;
        case USCI_UART_UCTXIFG: break;
        case USCI_UART_UCSTTIFG: break;
        case USCI_UART_UCTXCPTIFG: break;
        default: break;
    }
}

/*
 * UCA1 INTERRUPT (GPS UART)
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(UCA1IV, USCI_UART_UCTXCPTIFG)) {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
            buffer_write(&rx_buffers[UART_A1], UCA1RXBUF);
            break;
        case USCI_UART_UCTXIFG: break;
        case USCI_UART_UCSTTIFG: break;
        case USCI_UART_UCTXCPTIFG: break;
        default: break;
    }
}

/*
 * ============================================================================
 * USAGE EXAMPLES
 * ============================================================================
 * 
 * Example 1: Debug output
 * 
 *   uart_init(UART_A0, BAUD_115200);
 *   uart_write_string(UART_A0, "System started\r\n");
 * 
 * 
 * Example 2: Read GPS data
 * 
 *   uart_init(UART_A1, BAUD_9600);
 *   
 *   while (1) {
 *       if (uart_available(UART_A1) > 0) {
 *           uint8_t byte;
 *           if (uart_read_byte(UART_A1, &byte)) {
 *               // Process byte
 *           }
 *       }
 *   }
 * 
 * 
 * Example 3: Read line from UART
 * 
 *   char line[128];
 *   uint16_t len = uart_read_until(UART_A0, line, 128, '\n');
 *   if (len > 0) {
 *       line[len] = '\0';  // Null terminate
 *       // Process line
 *   }
 */
