#ifndef LORA_HANDLER_H
#define LORA_HANDLER_H

#include <Arduino.h>
#include <RadioLib.h>

// Define your PINs here so they are easy to change later

//C6
// #define LORA_CS     2
// #define LORA_DIO0   14  
// #define LORA_RST    RADIOLIB_NC           
// #define LORA_DIO1   RADIOLIB_NC

//C3
#define LORA_CS     4
#define LORA_DIO0   8
#define LORA_RST    RADIOLIB_NC            
#define LORA_DIO1   RADIOLIB_NC

class LoRaHandler {
public:
    // Constructor
    LoRaHandler();

    // Initialize the module. Returns true if successful.
    bool init();

    // Send a string message. Returns true if successful.
    bool send(String message);

private:
    // The actual RadioLib module object
    SX1276* radio;
};

#endif