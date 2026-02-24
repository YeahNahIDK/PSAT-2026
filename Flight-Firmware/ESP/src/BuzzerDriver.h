#ifndef BUZZER_DRIVER_H
#define BUZZER_DRIVER_H

#include <Arduino.h>

class BuzzerDriver {
public:
    BuzzerDriver(int pin);

    void begin();
    
    void on();  
    void off(); 
    void beep(int count, int durationMs = 100);
    void force_beep(int count, int durationMs);
    void update();

private:
    int _pin;
    bool _isActive;
    
    // Pattern variables
    int _beepCount;      
    int _beepDuration;   
    unsigned long _timer;
    bool _beepState;     
};

#endif