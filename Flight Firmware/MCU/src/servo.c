// servo.v - PEM SERVO CONTROL
//
// This servo expects a PWM signal:
//
//      Frequency: 50 Hz (period = 20 ms)
//      Pulse width:
//          1.0 ms = 0°
//          1.5 ms = 90°
//          2.0 ms = 180°
//
// The timer peripheral is used to generate this waveform.

void servo_init(){

    // Comfigure PWM output pin
    // Setup timer:
    //  - period = 20ms
    //  - initial pulse width = safe closed position

}

void servo_set_angle(int deg){

    // Convert deg (0-180) to microseconds (1000-2000)
    // Convert microseconds to timer counts
    // Update timer compare register 

}

void servo_deploy() {
    // Set a servo angle to release mechansim
    // Wait required time
}