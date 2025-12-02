// "pingpong.c" used as template (Found in external modules)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "delay.h"
#include "timer.h"
#include "radio.h"
#include "tremo_system.h"
#include "tremo_uart.h"
#include "tremo_rcc.h"
#include "tremo_gpio.h"

#define RF_FREQUENCY                                915000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       10         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT
}States_t;

#define RX_TIMEOUT_VALUE 1800
#define BUFFER_SIZE 128

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];
uint16_t RxBufferSize = 0;      // Size of received data (from OnRxDone)
uint16_t BufferIndex = 0;       // Current position when collecting UART data
uint16_t TxLength = 0;          // Size of data being transmitted

volatile States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );

/**
 * Main application entry point.
 */
int app_start( void )
{
    /* Serial iniialisation */

    // Enable clocks
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);

    // Init UART0
    gpio_set_iomux(GPIOB, GPIO_PIN_0, 1);
    gpio_set_iomux(GPIOB, GPIO_PIN_1, 1);

    // Config UART0
    uart_config_t uartConfig;
    uart_config_init(&uartConfig);
    uartConfig.baudrate = UART_BAUDRATE_9600;
    uart_init(UART0, &uartConfig);
    uart_cmd(UART0, ENABLE);

    /* Radio initialization */
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init( &RadioEvents );

    Radio.SetChannel( RF_FREQUENCY );

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

    Radio.Rx( RX_TIMEOUT_VALUE );

    while( 1 )
    {
        switch( State )
        {
        case RX:
            printf("Recieved: ");
            for (int i = 0; i < RxBufferSize; i++) {
                printf("%c", Buffer[i]);
                uart_send_data(UART0, Buffer[i]);
            }
            printf("\r\n");

            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case TX:
            printf("Sent: ");
            for (int i = 0; i < TxLength; i++) {
                printf("%c", Buffer[i]);
            }
            printf("\r\n");

            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case RX_TIMEOUT:
            printf("RX timed out\r\n");
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case RX_ERROR:
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case TX_TIMEOUT:
            printf("TX timed out\r\n");
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case LOWPOWER:
            if (uart_get_flag_status(UART0, UART_FLAG_RX_FIFO_EMPTY) == RESET) {
                Buffer[BufferIndex++] = uart_receive_data(UART0);
                if (Buffer[BufferIndex - 1] == '\n' || BufferIndex >= BUFFER_SIZE) {             
                    TxLength = BufferIndex;
                    Radio.Send(Buffer, TxLength);
                    BufferIndex = 0;
                }
            }
            break;
        default:
            // Set low power
            break;
        }

        // Process Radio IRQ
        Radio.IrqProcess( );
    }
}

void OnTxDone( void )
{
    printf(">>> OnTxDone called\r\n");
    Radio.Sleep( );
    State = TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    printf(">>> OnRxDone called: size=%d, rssi=%d, snr=%d\r\n", size, rssi, snr);
    if (rssi < -100 || snr < 0) {
        printf(">>> Signal too weak, ignoring\r\n");
        Radio.Sleep();
        Radio.Rx(RX_TIMEOUT_VALUE);
        return;
    }
    Radio.Sleep( );
    RxBufferSize = size;
    memcpy(Buffer, payload, RxBufferSize);
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
}

void OnTxTimeout( void )
{
    printf(">>> OnRxTimeout called\r\n");
    Radio.Sleep( );
    State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{
    printf(">>> OnRxTimeout called\r\n");
    Radio.Sleep( );
    State = RX_TIMEOUT;
}

void OnRxError( void )
{
    printf(">>> OnRxError called\r\n");
    Radio.Sleep( );
    State = RX_ERROR;
}
