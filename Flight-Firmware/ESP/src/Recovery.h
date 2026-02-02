#ifndef RECOVERY_LOGIC_H
#define RECOVERY_LOGIC_H

#include <Arduino.h>
#include "BuzzerDriver.h" // We need access to the buzzer to trigger alarms

// Function Prototype
// We pass the Buzzer by reference (&buzzer) so this logic can control it directly
void check_recovery_logic(bool apogee_reached, float currentAlt, bool baroValid, float gpsSpeed, bool gpsValid, BuzzerDriver &buzzer);

#endif