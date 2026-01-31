#include "LoRaHandler.h"

// Constructor: Initializes the RadioLib Module object
LoRaHandler::LoRaHandler() {
    // Create the generic Module object used by RadioLib
    radio = new SX1276(new Module(LORA_CS, LORA_DIO0, LORA_RST, LORA_DIO1));
}

bool LoRaHandler::init() {
    Serial.print("[LoRa] Initializing ... ");

    // 1. LoRa default settings:
    // Frequency: 915.0 MHz
    // Bandwidth: 125.0 kHz
    // Spreading Factor: 9
    // Coding Rate: 7
    // Sync Word: 0x12 (private network)
    // Output Power: 10 dBm
    // Preamble Length: 8 symbols
    // Amplifier Gain: 0 (automatic)
    int state = radio->begin(915.0); 

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("Success!");
        return true;
    } else {
        Serial.print("Failed, code ");
        Serial.println(state);
        return false;
    }
}

bool LoRaHandler::send(String message) {
    Serial.print("[LoRa] Transmitting packet ... ");

    // The transmit method blocks until transmission is finished
    int state = radio->transmit(message);

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("Success!");
        // Optional: print data rate
        // Serial.print("[LoRa] Datarate:\t");
        // Serial.print(radio->getDataRate());
        // Serial.println(" bps");
        return true;
    } else if (state == RADIOLIB_ERR_PACKET_TOO_LONG) {
        Serial.println("Too long!");
    } else if (state == RADIOLIB_ERR_TX_TIMEOUT) {
        Serial.println("Timeout!");
    } else {
        Serial.print("Failed, code ");
        Serial.println(state);
    }
    return false;
}