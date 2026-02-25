#include <iostream>
#include <cmath>
#include <iomanip>
#include <string>

using std::abs;

/* === Mock Hardware Dependencies === */
unsigned long mock_millis_val = 0;
unsigned long millis() { return mock_millis_val; }

/* === Configuration Constants === */
const float STABILITY_MAX_DRIFT = 2.0f;    // meters
const unsigned long STABILITY_DETECT_MS = 3000;
const float GPS_STABLE_MAX_SPEED = 2.0f;   // km/h

/* === Mocking Global State === */
struct TelemetryData {
    float altitude = 0.0f;
};
TelemetryData flight;

// Mocking TinyGPS++ structure
struct MockSpeed {
    float current_speed = 0.0f;
    float kmph() { return current_speed; }
};
struct MockTGps {
    MockSpeed speed;
};
struct MockGPS {
    bool is_valid = true;
    MockTGps tGps;
    bool isValid() { return is_valid; }
} gps;

// Extracted from 'static' for resetting and printing
float anchorAltitude = 0.0f;        
unsigned long stableStartTime = 0;

void reset_stability_state() {
    mock_millis_val = 0;
    flight.altitude = 0.0f;
    gps.is_valid = true;
    gps.tGps.speed.current_speed = 0.0f;
    anchorAltitude = 0.0f;
    stableStartTime = 0;
}


bool stability_check() {
    bool gpsMoving = gps.isValid() && (gps.tGps.speed.kmph() > GPS_STABLE_MAX_SPEED);

    if (stableStartTime == 0 || abs(flight.altitude - anchorAltitude) > STABILITY_MAX_DRIFT) {
        anchorAltitude = flight.altitude;
        stableStartTime = millis(); 
        
        if (gpsMoving) {
            stableStartTime = 0; 
        }
    }
    else {
        if (stableStartTime != 0 && (millis() - stableStartTime > STABILITY_DETECT_MS)) {
            return true;
        }
    }

    return false;
}


/* === Test Runner === */
struct TestData {
    float alt;
    float speed;
    bool gps_valid;
};

void run_mock_landing(const std::string& test_name, TestData* profile, int num_samples) {
    reset_stability_state();
    std::cout << "\n============================================================" << std::endl;
    std::cout << "TEST FLIGHT: " << test_name << std::endl;
    std::cout << "============================================================" << std::endl;
    
    for (int i = 0; i < num_samples; i++) {
        flight.altitude = profile[i].alt;
        gps.tGps.speed.current_speed = profile[i].speed;
        gps.is_valid = profile[i].gps_valid;
        
        mock_millis_val += 500; // 500ms per loop

        bool is_landed = stability_check();
        
        long elapsed = (stableStartTime == 0) ? 0 : (mock_millis_val - stableStartTime);

        std::cout << "Time: " << std::setw(4) << mock_millis_val << "ms "
                  << "| Alt: " << std::setw(4) << flight.altitude << "m "
                  << "| Spd: " << std::setw(4) << gps.tGps.speed.kmph() << "km/h "
                  << "| Anchor: " << std::setw(4) << anchorAltitude << "m "
                  << "| Timer: " << std::setw(4) << elapsed << "ms "
                  << "| Landed? " << (is_landed ? "YES <--- TRIGGERED!" : "No") 
                  << std::endl;
    }
}

int main() {
    /*
        TEST 1: The Clean Landing
        Rocket drops to 0m, speed drops to 0 km/h. Timer should start and trigger at 3000ms.
    */
    TestData clean_landing[] = {
        {15.0, 20.0, true}, {10.0, 15.0, true}, {5.0, 10.0, true}, // Descending
        {0.0,  0.0,  true}, {0.1,  0.0,  true}, {-0.1, 0.0,  true}, // Landed, minor noise
        {0.0,  0.0,  true}, {0.1,  0.0,  true}, {0.0,  0.0,  true}, // Timer counts up
        {0.0,  0.0,  true}, {0.0,  0.0,  true}, {0.0,  0.0,  true}
    };
    
    /*
        TEST 2: High Wind Dragging
        Rocket is on the ground (altitude stable), but wind is dragging the parachute (high GPS speed).
        Timer should stay at 0 until speed drops below 2.0 km/h.
    */
    TestData wind_dragging[] = {
        {0.0, 15.0, true}, {0.1, 12.0, true}, {0.0, 8.0, true},  // Dragging fast
        {-0.1, 5.0, true}, {0.0, 3.0, true},  {0.1, 1.0, true},  // Slowing down (timer starts at 1.0)
        {0.0,  0.0, true}, {0.0, 0.0, true},  {0.0, 0.0, true},  // Stopped
        {0.0,  0.0, true}, {0.0, 0.0, true},  {0.0, 0.0, true}
    };

    /*
        TEST 3: The Tree Landing / Barometric Drift
        Speed is 0, but altitude drifts past the 2.0m threshold (e.g. branch breaks and it drops 3 meters).
        The timer should reset halfway through.
    */
    TestData tree_landing[] = {
        {10.0, 0.0, true}, {10.1, 0.0, true}, {10.0, 0.0, true}, // Caught in tree
        {9.9,  0.0, true}, {10.0, 0.0, true}, {10.1, 0.0, true}, // Timer reaches 2500ms
        {6.5,  0.0, true}, // BRANCH BREAKS! Drops 3.5m (exceeds 2.0m drift) -> Timer MUST reset
        {6.6,  0.0, true}, {6.5,  0.0, true}, {6.5,  0.0, true}, // Timer starts again
        {6.5,  0.0, true}, {6.5,  0.0, true}, {6.5,  0.0, true}, {6.5, 0.0, true}
    };

    run_mock_landing("1. Clean Landing", clean_landing, sizeof(clean_landing)/sizeof(clean_landing[0]));
    run_mock_landing("2. High Wind Dragging", wind_dragging, sizeof(wind_dragging)/sizeof(wind_dragging[0]));
    run_mock_landing("3. Tree Landing / Barometric Drift", tree_landing, sizeof(tree_landing)/sizeof(tree_landing[0]));

    return 0;
}