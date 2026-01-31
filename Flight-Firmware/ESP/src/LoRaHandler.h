#ifndef LORA_HANDLER_H
#define LORA_HANDLER_H

#include <Arduino.h>
#include <RadioLib.h>

// Define your PINs here so they are easy to change later
#define LORA_CS     10
#define LORA_DIO0   2
#define LORA_RST    3
#define LORA_DIO1   4

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