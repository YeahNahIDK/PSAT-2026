#ifndef RECOVERY_LOGIC_H
#define RECOVERY_LOGIC_H

#include <Arduino.h>
#include "BuzzerDriver.h" 
#include "BMP390Driver.h" // <--- Include your new driver

// Pass the drivers by reference
void check_recovery_logic(bool apogee_reached, 
                          BMP390Driver &altimeter, 
                          float gpsSpeed, 
                          bool gpsValid, 
                          BuzzerDriver &buzzer);

#endif