#include <iostream>
#include <cmath>
#include <iomanip>
#include <string>

/* === Mock Hardware Dependencies === */
unsigned long mock_millis_val = 0;
unsigned long millis() { return mock_millis_val; }


/* === Configuration Constants === */
const float APOGEE_MAX_VELOCITY = 350.0f; // m/s
const float APOGEE_DROP_THRESHOLD = 2.0f; // meters
const int APOGEE_DETECT_COUNT = 5;        // consecutive readings


/* === Global State === */
struct TelemetryData {
    float altitude = 0.0f;
    float max_altitude = 0.0f;
};
TelemetryData flight;

// Extracted from 'static' to global for reset between flights
int below_max_count = 0;
unsigned long last_update = 0;
float prev_altitude = 0.0f;


void reset_apogee_state() {
    mock_millis_val = 0;
    flight.altitude = 0.0f;
    flight.max_altitude = 0.0f;
    below_max_count = 0;
    last_update = 0;
    prev_altitude = 0.0f;
}


bool apogee_detect() {
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

    /* Sanity Check */
    if (std::abs(velocity) < APOGEE_MAX_VELOCITY) {
        /* Set Maximum Altitude */
        if (flight.altitude > flight.max_altitude) {
            flight.max_altitude = flight.altitude;
            below_max_count = 0;
        } else if ((flight.max_altitude - flight.altitude) >= APOGEE_DROP_THRESHOLD){
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
void run_mock_flight(const std::string& test_name, float* profile, int num_samples) {
    float millis_increase = 100;

    reset_apogee_state();
    std::cout << "\n============================================================" << std::endl;
    std::cout << "TEST FLIGHT: " << test_name << std::endl;
    std::cout << "============================================================" << std::endl;
    
    for (int i = 0; i < num_samples; i++) {
        flight.altitude = profile[i];
        mock_millis_val += millis_increase; // 100ms per loop
        
        float vel = 0.0f;
        if (i > 0) vel = (profile[i] - profile[i-1]) / (millis_increase / 1000);

        bool is_apogee = apogee_detect();

        std::cout << "Time: " << std::setw(4) << mock_millis_val << "ms "
                  << "| Alt: " << std::setw(5) << flight.altitude << "m "
                  << "| Vel: " << std::setw(5) << vel << "m/s "
                  << "| MaxAlt: " << std::setw(5) << flight.max_altitude << "m "
                  << "| Count: " << below_max_count << "/5 "
                  << "| Apogee? " << (is_apogee ? "YES <--- TRIGGERED!" : "No") 
                  << std::endl;
    }
}

int main() {
    /*
        TEST 1: The Transonic Spike (Sanity Check test)
        Suddenly jumping from 50m to 450m in 100ms is 4000 m/s. The logic should ignore it.
    */
    float flight_spike[] = {0, 10, 20, 30, 40, 50, 450, 60, 70, 80, 75, 70, 65, 60, 55, 50};
    
    /*
        TEST 2: The Double Peak (Staging / Engine stutter test)
        Goes up to 100m, drops past the threshold (starting the counter), then main engine blasts it to 200m.
        The counter must completely reset to 0 and track the new peak.
    */
    float flight_double_peak[] = {0, 50, 100, 97, 95, 93, 100, 120, 118, 115, 113, 110, 108, 105};
    
    /*
        TEST 3: The Edge of Threshold (Jitter test)
        Hovering exactly on the boundary of the 2.0m threshold. It dips below, starts counting, 
        but then pops back above the threshold drop. The counter must reset.
    */
    float flight_jitter[] = {0, 20, 40, 60, 80, 100, 99.0, 98.1, 97.9, 98.5, 98.0, 97.5, 97.0, 96.0, 95.0, 94.0};

    run_mock_flight("1. Velocity Spike (Sanity Check)", flight_spike, sizeof(flight_spike)/sizeof(flight_spike[0]));
    run_mock_flight("2. Double Peak (Reset Count)", flight_double_peak, sizeof(flight_double_peak)/sizeof(flight_double_peak[0]));
    run_mock_flight("3. Threshold Jitter (Reset Count)", flight_jitter, sizeof(flight_jitter)/sizeof(flight_jitter[0]));

    return 0;
}