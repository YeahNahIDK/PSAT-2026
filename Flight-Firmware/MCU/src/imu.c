
// imu.c — IMPLEMENTATION 
// BNO085 RVC packet structure (as shown in datasheet):
//   [Header][yaw][pitch][roll][ax][ay][az]
// Data is little-endian, scaled into degrees and m/s^2.

static float yaw, pitch, roll;
static float ax, ay, az;

void imu_init() {
    // UART @ 115200 baud
    // Pin P0 must be pulled HIGH to activate RVC mode (from datasheet)
}

int imu_read() {
    // UART3: read bytes into a 20-byte buffer
    // Once all bytes arrive:
    //     decode packet then update yaw, pitch, roll, ax, ay, az
    //     return 1
    // else return 0
}

float imu_yaw()  { return yaw; }
float imu_pitch(){ return pitch; }
float imu_roll() { return roll; }

float imu_ax(){ return ax; }
float imu_ay(){ return ay; }
float imu_az(){ return az; }
