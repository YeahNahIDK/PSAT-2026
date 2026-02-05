#include "Recovery.h"
#include "esp_log.h"

static const char *TAG = "RECOVERY";

// --- CONFIGURATION ---
#define LANDING_DETECT_MS       10000   // 10 Seconds stable
#define ALARM_START_DELAY_MS    (15 * 60 * 1000) // 15 Minutes
#define MAX_GPS_SPEED_KMPH      15.0    

RecoveryState check_recovery_logic(bool apogee_reached, BMP390Driver &altimeter, float gpsSpeed, bool gpsValid, BuzzerDriver &buzzer) {
    
    // --- STATE VARIABLES ---
    static RecoveryState currentState = REC_IN_AIR;
    
    static unsigned long lastCheckTime = 0;
    static float anchorAltitude = 0;        
    static unsigned long stableStartTime = 0;
    static unsigned long landedTimestamp = 0;
    static unsigned long lastBeep = 0;

    if (!apogee_reached) return currentState;

    if (millis() - lastCheckTime < 1000) return currentState;
    
    lastCheckTime = millis();

    float currentAlt = altimeter.getAltitude();

    switch (currentState) {
        case REC_IN_AIR: {
            bool gpsMoving = gpsValid && (gpsSpeed > MAX_GPS_SPEED_KMPH);
            
            // LOGIC: Displacement Method
            // If timer not started, OR we drifted more than 2 meters from anchor...
            if (stableStartTime == 0 || abs(currentAlt - anchorAltitude) > 2.0) {
                
                // Reset Anchor
                anchorAltitude = currentAlt;
                stableStartTime = millis(); 
                
                // If GPS says we are moving fast, force a hard reset (0)
                if (gpsMoving) {
                    stableStartTime = 0; 
                }
            }
            else {
                // We are hovering within +/- 2m of anchor.
                // Check if we have been here long enough
                if (stableStartTime != 0 && (millis() - stableStartTime > LANDING_DETECT_MS)) {
                    currentState = REC_LANDED;
                    landedTimestamp = millis();
                    ESP_LOGE(TAG, ">>> LANDING CONFIRMED (Alt: %.1fm) <<<", currentAlt);
                    buzzer.beep(3, 500); 
                }
            }
            break; 
        }

        case REC_LANDED:
            if (millis() - landedTimestamp > ALARM_START_DELAY_MS) {
                if (millis() - lastBeep > 3000) {
                    lastBeep = millis();
                    ESP_LOGI(TAG, "Recovery Mode Beep");
                    buzzer.beep(2, 1000); 
                }
            }
            break;
    }

    return currentState;
}