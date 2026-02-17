#include "LoRaHandler.h"

// Constructor: Initializes the RadioLib Module object
LoRaHandler::LoRaHandler() {
    // Create the generic Module object used by RadioLib
    radio = new SX1276(new Module(LORA_CS, LORA_DIO0, LORA_RST, LORA_DIO1, SPI));
}

bool LoRaHandler::init() {
    Serial.print("[LoRa] Initializing ... ");

    // begin(Freq, Bandwidth, SF, CodingRate, SyncWord, Power, Preamble, Gain)  
    // 1. Frequency: 915.0 MHz
    // 2. Bandwidth: 125.0 kHz (Matches index 0)
    // 3. Spreading Factor: 8
    // 4. Coding Rate: 5 (Matches index 1 which is 4/5)
    // 5. Sync Word: 0x12 (Standard Private Network)
    // 6. Power: 14 dBm
    // 7. Preamble: 8
    // 8. Gain: 0 (Auto)
    int state = radio->begin(915.0, 125.0, 10, 5, 0x12, 14, 8, 0);

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