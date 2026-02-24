#include "LoRaHandler.h"
#include "hardware_config.h"

// Constructor: Initializes the RadioLib Module object
LoRaHandler::LoRaHandler() {
    // Create the generic Module object used by RadioLib
    radio = new SX1276(new Module(PIN_LORA_CS, PIN_LORA_DIO0, PIN_LORA_RST, PIN_LORA_DIO1, SPI));
}

bool LoRaHandler::begin() {
    Serial.print("[LoRa] Initializing ... ");
    
    int state = radio->begin(LORA_FREQUENCY, LORA_BANDWIDTH, LORA_SF,
        LORA_CR, LORA_SYNC_WORD, LORA_POWER, LORA_PREAMBLE, LORA_GAIN);

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