
// state_machine.c 

//   - Detect APogee using barometric altitude.
//   - Once APOGEE is detected:
//           Start the servo (to spin the streamer)
//   - After apogee, rocket enters DESCENT.
//   - When altitude gets very low, mark rocket as LANDED.
//
// Notes:
//   - GPS is only used to confirm we have a fix before flight.
//   - All altitude-based logic uses barometric altitude.
//   - Servo is only activated ONCE (at apogee) and not used again.

FlightState current_state;
float previous_altitude = 0.0f;
int decreasing_counter = 0;     // counts consecutive decreases in altitude
int servo_started = 0;          // ensures servo spins only once

void state_init(void) {
    current_state = STATE_STANDBY;
    previous_altitude = 0.0f;
    decreasing_counter = 0;
    servo_started = 0;
}

void state_update(void) {

    float alt = baro_altitude();   // read barometric altitude

    switch (current_state) {

 
        case STATE_STANDBY: {
      -
            // Rocket is sitting on the pad.
            // Only move to ASCENT once GPS has a valid lock.
            if (gps_has_fix()) {
                current_state = STATE_ASCENT;
            }
            break;
        }

     
        case STATE_ASCENT: {
    
            // Check if altitude stops increasing then possible apogee.
            if (alt < previous_altitude) {
                decreasing_counter++;
            } else {
                decreasing_counter = 0;
            }

            // Require multiple decreases to confirm trend.
            if (decreasing_counter > 5) {
                current_state = STATE_APOGEE;
            }
            break;
        }


        case STATE_APOGEE: {
  
            // At apogee:
            // Start the servo ONE TIME ONLY to spin the streamer.
            if (servo_started == 0) {
                servo_start_spin();   // <--- your function to run motor/servo
                servo_started = 1;    // prevents running again
            }

            // Then transition to descent.
            current_state = STATE_DESCENT;
            break;
        }


        case STATE_DESCENT: {
 
            // Servo is already spinning from APOGEE.
            // No additional actions needed.

            // Detect landing using low altitude.
            if (alt < 5.0f) {
                current_state = STATE_LANDED;
            }
            break;
        }

   
        case STATE_LANDED: {
       
            // Rocket is on the ground.
            // Everything else (logging, telemetry) continues normally.
            break;
        }
    }

    // Save altitude for next loop comparison.
    previous_altitude = alt;
}

FlightState state_get(void) {
    return current_state;
}
