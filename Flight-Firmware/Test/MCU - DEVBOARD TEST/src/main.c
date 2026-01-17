#include <msp430.h>
#include <stdint.h>

// ============================================================================
// WIRING FOR MSP430FR2433
// ============================================================================
// LoRa TX: P2.6  -> RA-08H RX  (HARDWARE UART A1)
// LoRa RX: P2.5  -> RA-08H TX  (HARDWARE UART A1)
// Debug:   P2.0  -> PC         (SOFTWARE UART)
// ============================================================================

#define UART_DELAY_8MHZ 833 
volatile uint32_t g_system_tick = 0;

void clock_init_8mhz(void) {
    FRCTL0 = FRCTLPW | NWAITS_1;
    __bis_SR_register(SCG0); CSCTL3 = SELREF__REFOCLK; CSCTL0 = 0;
    CSCTL1 &= ~(DCORSEL_7); CSCTL1 |= DCORSEL_3; CSCTL2 = FLLD_0 + 243;
    __delay_cycles(3); __bic_SR_register(SCG0);
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;
}

// Debug on P2.0 (Software) - Keeps you informed
void debug_print(const char* str) {
    while (*str) {
        char c = *str++;
        __disable_interrupt();
        P2OUT &= ~BIT0; __delay_cycles(UART_DELAY_8MHZ);
        for (int i=0; i<8; i++) {
            if (c & 1) P2OUT |= BIT0; else P2OUT &= ~BIT0;
            c >>= 1; __delay_cycles(UART_DELAY_8MHZ);
        }
        P2OUT |= BIT0; __delay_cycles(UART_DELAY_8MHZ);
        __enable_interrupt();
    }
}

// LoRa on P2.5/P2.6 (Hardware UCA1)
void lora_init_uca1(void) {
    // Correct Pins for MSP430FR2433 UCA1
    P2SEL0 |= BIT5 | BIT6;  // P2.5 = RX, P2.6 = TX
    P2SEL1 &= ~(BIT5 | BIT6);
    
    UCA1CTLW0 = UCSWRST;
    UCA1CTLW0 |= UCSSEL__SMCLK; // 8MHz
    
    // 9600 Baud @ 8MHz
    UCA1BRW = 52;
    UCA1MCTLW = (0x49 << 8) | (1 << 4) | UCOS16;
    
    UCA1CTLW0 &= ~UCSWRST;
    // We can enable interrupts later if needed
}

void lora_send(const char* str) {
    while (*str) {
        while(!(UCA1IFG & UCTXIFG)); // Wait for HW Buffer
        UCA1TXBUF = *str++;
    }
}

void system_init(void) {
    WDTCTL = WDTPW | WDTHOLD; PM5CTL0 &= ~LOCKLPM5;
    clock_init_8mhz();
    TA0CCR0 = 8000 - 1; TA0CTL = TASSEL__SMCLK | MC__UP; TA0CCTL0 = CCIE;
    __enable_interrupt();
}

int main(void) {
    system_init();
    P1DIR |= BIT0; P1OUT &= ~BIT0; // LED
    P2DIR |= BIT0; P2OUT |= BIT0;  // P2.0 Debug
    
    lora_init_uca1();
    
    // Startup Delay
    for(volatile long i=0; i<100000; i++);
    debug_print("=== MSP430 UCA1 (P2.6) MODE ===\r\n");

    uint32_t last_send = 0;
    while (1) {
        if (g_system_tick - last_send >= 3000) {
            P1OUT ^= BIT0; // Flash LED
            
            // Send to LoRa (Hardware P2.6)
            lora_send("Ping\r\n");
            
            // Send to PC (Software P2.0)
            debug_print("Sent: Ping\r\n");
            
            last_send = g_system_tick;
        }
    }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A0_ISR (void)
#endif
{ g_system_tick++; }
