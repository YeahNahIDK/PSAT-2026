#ifndef LORA_HANDLER_H
#define LORA_HANDLER_H

#include <Arduino.h>
#include <RadioLib.h>

class LoRaHandler {
public:
    // Constructor
    LoRaHandler();

    // Initialize the module. Returns true if successful.
    bool begin();

    // Send a string message. Returns true if successful.
    bool send(String message);

private:
    // The actual RadioLib module object
    SX1276* radio;
};

#endif