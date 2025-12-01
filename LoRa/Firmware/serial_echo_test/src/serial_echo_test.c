#include "tremo_uart.h"
#include "tremo_rcc.h"
#include "tremo_gpio.h"

int app_start( void )
{
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

    while( 1 ) {
        if (uart_get_flag_status(UART0, UART_FLAG_RX_FIFO_EMPTY) == RESET) {
            uart_send_data(UART0, uart_receive_data(UART0));
        }      
    }
}
