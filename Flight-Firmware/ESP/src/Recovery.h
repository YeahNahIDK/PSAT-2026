#ifndef RECOVERY_LOGIC_H
#define RECOVERY_LOGIC_H

#include <Arduino.h>
#include "BuzzerDriver.h" 
#include "BMP390Driver.h"

enum RecoveryState {
    REC_PRE_LAUNCH,
    REC_IN_AIR,
    REC_LANDED
};

RecoveryState check_recovery_logic(bool apogee_reached, 
                          BMP390Driver &altimeter, 
                          float gpsSpeed, 
                          bool gpsValid, 
                          BuzzerDriver &buzzer);

#endif