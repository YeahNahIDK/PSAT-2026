// lora.c - LORA AT-COMMAND INTERFACE 
//
// The RA-08H module alredy contains LoRA radio firmware.
// This .c file sends ASCII AT commands thorugh UART.
//
// e.g., 
//      AT+SEND=20,HELLO_FROM_ROCKET

void lora_init(){
    
    // Setup UART at 9600 baud
    // Send "AT" and wait for "OK"
    // Set region frequency (915 MHz for NZ)
    // Set TX power
    // (i think) Disable LoRaWAN mode (transparent LoRa mode)

}

void lora_send(char *payload){

    // Calculate payload length
    // Build a string in the format;
    //          "AT+SEND=<length>, <payload>\r\n"
    // Send this over UART
}