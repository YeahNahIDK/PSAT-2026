// servo.h - SERVO CONTROL DECLARATIONS
//
// Used to control (i forgot what model) deployment servo.

void servo_init();                 // Configure PWM at 50 Hz
void servo_set_angle(int degrees)  // Move servo to angle (0-180)
void servo-deploy();               // Move servo to deployment position