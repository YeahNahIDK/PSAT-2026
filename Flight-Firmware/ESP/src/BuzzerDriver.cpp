#include "BuzzerDriver.h"
#include "esp_log.h"

// CMT-8504 Resonance Freq
#define BUZZER_FREQ 4000 
#define BUZZER_RES  8    // 8-bit resolution (0-255)

static const char *TAG = "BUZZER";

BuzzerDriver::BuzzerDriver(int pin) 
    : _pin(pin), _isActive(false), _beepCount(0), _beepState(false) {
}

void BuzzerDriver::begin() {
    if (!ledcAttach(_pin, BUZZER_FREQ, BUZZER_RES)) {
        ESP_LOGE(TAG, "Failed to attach LEDC to pin %d", _pin);
        return;
    }
    
    off();
    ESP_LOGI(TAG, "Initialized on Pin %d, Freq %dHz (v3.0)", _pin, BUZZER_FREQ);
}

void BuzzerDriver::on() {
    uint32_t duty = (1 << BUZZER_RES) / 2;
    
    ledcWrite(_pin, duty); // 50% duty cycle
    _isActive = true;
}

void BuzzerDriver::off() {
    ledcWrite(_pin, 0);
    _isActive = false;
}

void BuzzerDriver::beep(int count, int durationMs) {
    _beepCount = count * 2; 
    _beepDuration = durationMs;
    _timer = millis();
    _beepState = false; 
    
    on();
    _beepState = true;
    _beepCount--; 
}

void BuzzerDriver::update() {
    if (_beepCount > 0) {
        if (millis() - _timer >= _beepDuration) {
            _timer = millis();
            
            if (_beepState) {
                off();
                _beepState = false;
            } else {
                on();
                _beepState = true;
            }
            
            _beepCount--;
        }
    }
}

void BuzzerDriver::force_beep(int count, int durationMs) {
    for (int i = 0; i < count; i++) {
        on();
        delay(durationMs);
        
        off();
        delay(durationMs);
    }
    
    // Prevents clash with update()
    _beepCount = 0; 
    _beepState = false;
}
