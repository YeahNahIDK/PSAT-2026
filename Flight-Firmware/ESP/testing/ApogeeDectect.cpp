#include <iostream>
#include <cmath>
#include <iomanip>

/* === Mock Hardware Dependencies === */
unsigned long mock_millis_val = 0;
unsigned long millis() {
    return mock_millis_val;
}

/* === Configuration Constants === */
const float APOGEE_MAX_VELOCITY = 350.0f; // m/s
const float APOGEE_DROP_THRESHOLD = 2.0f; // meters
const int APOGEE_DETECT_COUNT = 5;        // consecutive readings

/* === Mock Global State === */
struct TelemetryData {
    float altitude = 0.0f;
    float max_altitude = 0.0f;
};
TelemetryData flight;


bool apogee_detect() {
    static unsigned long last_update = 0;
    static float prev_altitude = 0.0f;

    if (last_update == 0) {
        last_update = millis();
        prev_altitude = flight.altitude;
        flight.max_altitude = flight.altitude;
        return false;
    }

    unsigned long current_time = millis();
    unsigned long delta_time = current_time - last_update;
    if (delta_time == 0) return false;

    float delta_altitude = flight.altitude - prev_altitude;
    float velocity = delta_altitude / (delta_time / 1000.0f);

    static int below_max_count = 0;

    /* Sanity Check */
    if (std::abs(velocity) < APOGEE_MAX_VELOCITY) {
        /* Set Maximum Altitude */
        if (flight.altitude > flight.max_altitude) {
            flight.max_altitude = flight.altitude;
            below_max_count = 0;
        } else if ((flight.max_altitude - flight.altitude) > APOGEE_DROP_THRESHOLD){
            below_max_count += 1;
        } else {
            below_max_count = 0;  // Handles jitter
        }

        /* Set Apogee */
        if (below_max_count >= APOGEE_DETECT_COUNT) {
            return true;
        }
    }

    prev_altitude = flight.altitude;
    last_update = current_time;

    return false;
}


/* === Test Runner === */
int main() {
    float mock_flight_profile[] = {
        0.0, 20.0, 50.0, 80.0, 95.0, 99.0, 100.0, // Ascending cleanly
        99.5, 98.5, 99.8, 98.2,                   // JITTER AT PEAK (Should NOT trigger)
        97.0, 95.0, 92.0, 89.0, 85.0, 80.0, 75.0  // Descending cleanly (Should trigger)
    };
    
    int num_samples = sizeof(mock_flight_profile) / sizeof(mock_flight_profile[0]);

    std::cout << "--- Apogee Detection Simulation ---" << std::endl;

    for (int i = 0; i < num_samples; i++) {
        flight.altitude = mock_flight_profile[i];
        mock_millis_val += 100; // 100ms per loop

        bool is_apogee = apogee_detect();

        std::cout << "Time: " << std::setw(4) << mock_millis_val << "ms "
                  << "| Alt: " << std::setw(5) << flight.altitude << "m "
                  << "| Max Alt: " << std::setw(5) << flight.max_altitude << "m "
                  << "| Apogee? " << (is_apogee ? "YES <--- TRIGGERED!" : "No") 
                  << std::endl;
    }

    return 0;
}