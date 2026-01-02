// state_machine.h - FLIGHT STATE DECLARATIONS 
//
// Define rocket flight states:
//      IDLE - ASCENT - APOGEE - DESCENT - LANDED

typedef enum {
    STATE_IDLE, 
    STATE_ASCENT, 
    STATE_APOGEE,
    STATE_DESCENT,
    STATE_LANDED
} FlightState;

void state_init();
void state_update();
FlightState state_get();
