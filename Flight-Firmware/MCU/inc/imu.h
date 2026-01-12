// imu.h — BNO085 IMU (AHRS) IN UART RVC MODE (i forgor we werent using uart so need to change)

// The BNO085 outputs orientation as yaw-pitch-roll and acceleration
// in a simple, fixed-length binary packet when placed in RVC mode.

void imu_init();    
int imu_read();     // Returns 1 when a full packet is received

float imu_yaw();
float imu_pitch();
float imu_roll();

float imu_ax();
float imu_ay();
float imu_az();
