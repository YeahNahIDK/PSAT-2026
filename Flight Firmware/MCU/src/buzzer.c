// Active buzzer used - HIGH results in sound

void buzzer_init() {
    P2DIR = 0x01; // Pin 2.0 output, rest input
    P2OUT = 0x00; // Set output to low for all states
}

void buzzer_on() {
    // Digital write HIGH
    P2OUT = 0x01;
}

void buzzer_off() {
    // Digital write LOW
    P2OUT = 0x00;
}