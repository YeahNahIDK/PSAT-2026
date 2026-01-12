// lora.h - LORA FUNCTION DECLARATIONS
//
// This module provides:
//      - lora_init(): configure module with AT commands
//      - lora_send(): transmit ASCII telemetry payload

void lora_init();               // Setup UART + send AT configuration commands
void lora_send(char *payload);  // Sends message vis "AT+SEND"
