#include "BuzzerDriver.h"
#include "esp_log.h"
#include <Arduino.h>

// CMT-8504 Resonance Freq
#define BUZZER_FREQ                 4000 
#define BUZZER_TIMER_RESOLUTION     8
#define BUZZER_TIMER_CHANNEL        5
#define BUZZER_DUTY_CYCLE           128  // 50% duty cycle creates the loudest square wave.

static const char *TAG = "BUZZER";

BuzzerDriver::BuzzerDriver(int pin) 
    : _pin(pin), _isActive(false), _beepCount(0), _beepState(false) {
}

void BuzzerDriver::begin() {
    ledcAttachChannel(_pin, BUZZER_FREQ, BUZZER_TIMER_RESOLUTION, BUZZER_TIMER_CHANNEL);
    off();
    
    ESP_LOGI(TAG, "Initialized on Pin %d, Freq %dHz (LEDC API)", _pin, BUZZER_FREQ);
}

void BuzzerDriver::on() {
    ledcWrite(_pin, BUZZER_DUTY_CYCLE); 
    _isActive = true;
}                                                                                       

void BuzzerDriver::off() {
    // 0 duty cycle mutes the buzzer
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
