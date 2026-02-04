#include "Recovery.h"
#include "esp_log.h"

static const char *TAG = "RECOVERY";

// --- CONFIGURATION ---
#define LANDING_DETECT_MS       10000   // 10 Seconds stable
#define ALARM_START_DELAY_MS    (15 * 60 * 1000) // 15 Minutes

#define MAX_GPS_SPEED_KMPH      15.0    
#define VERTICAL_SPEED_THRESH   0.5     // +/- 0.5 m/s

// --- STATE TRACKING ---
enum State { IN_AIR, LANDED };
static State currentState = IN_AIR;

void check_recovery_logic(bool apogee_reached, BMP390Driver &altimeter, float gpsSpeed, bool gpsValid, BuzzerDriver &buzzer) {
    
    static unsigned long lastCheckTime = 0;
    static float lastAltitude = 0;
    static unsigned long stableStartTime = 0;
    static unsigned long landedTimestamp = 0;
    static unsigned long lastBeep = 0;

    // 1. Safety Gate: Don't check for landing if we haven't even reached apogee
    if (!apogee_reached) return;

    // 2. Frequency Gate: Run approx 1Hz
    if (millis() - lastCheckTime < 1000) return;

    // 3. READ SENSOR
    float currentAlt = altimeter.getAltitude();
    
    // Simple validity check: If it returns exactly 0.0, it usually means read failure 
    bool baroValid = (currentAlt != 0.0);

    if (!baroValid) {
        ESP_LOGW(TAG, "Baro invalid (0.0) - Landing logic PAUSED");
        stableStartTime = 0; 
        lastCheckTime = millis();
        return;
    }

    // 4. Calculate Vertical Speed
    float timeDeltaSec = (millis() - lastCheckTime) / 1000.0;
    float verticalSpeed = (currentAlt - lastAltitude) / timeDeltaSec;
    
    lastAltitude = currentAlt; 
    lastCheckTime = millis();

    switch (currentState) {
        case IN_AIR: {
            bool isVerticallyStable = abs(verticalSpeed) < VERTICAL_SPEED_THRESH;
            bool isMovingFast = gpsValid && (gpsSpeed > MAX_GPS_SPEED_KMPH);
            
            // LOGIC: Stable Vertical AND Not moving fast horizontally
            if (isVerticallyStable && !isMovingFast) {
                if (stableStartTime == 0) {
                    stableStartTime = millis();
                    ESP_LOGI(TAG, "Stability Detected (VSpeed: %.2fm/s). Timer started.", verticalSpeed);
                }
                
                if (millis() - stableStartTime > LANDING_DETECT_MS) {
                    currentState = LANDED;
                    landedTimestamp = millis();
                    ESP_LOGE(TAG, ">>> LANDING CONFIRMED <<<");
                    buzzer.beep(3, 500); 
                }
            } else {
                if (stableStartTime != 0) {
                    ESP_LOGW(TAG, "Movement detected (VSpeed: %.2f). Resetting timer.", verticalSpeed);
                }
                stableStartTime = 0;
            }
            break; 
        }

        case LANDED:
            if (millis() - landedTimestamp > ALARM_START_DELAY_MS) {
                if (millis() - lastBeep > 3000) {
                    lastBeep = millis();
                    ESP_LOGI(TAG, "Recovery Mode Activated");
                    buzzer.beep(2, 1000); 
                }
            }
            break;
    }
}