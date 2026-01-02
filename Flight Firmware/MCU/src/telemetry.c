// telemetry.c 
// Builds a single CSV line with:
//   - timestamp
//   - GPS lat/lon/alt + fix
//   - barometer altitude, pressure, temperature
//   - IMU yaw, pitch, roll, ax, ay, az
//   - current flight state (STANDBY / ASCENT / APOGEE / DESCENT / LANDED)


void telemetry_format(char *out) {

    unsigned long t  = millis();

    float lat    = gps_lat();
    float lon    = gps_lon();
    float gpsAlt = gps_alt();
    int   fix    = gps_has_fix();

    float barAlt = baro_altitude();
    float press  = baro_pressure();
    float temp   = baro_temperature();

    float yaw   = imu_yaw();
    float pitch = imu_pitch();
    float roll  = imu_roll();

    float ax = imu_ax();
    float ay = imu_ay();
    float az = imu_az();

    int state = state_get();

    sprintf(out,
        "%lu,"            // timestamp
        "%.6f,%.6f,"      // lat, lon
        "%.2f,%d,"        // gps altitude, fix
        "%.2f,%.2f,%.2f," // baro altitude, pressure, temperature
        "%.2f,%.2f,%.2f," // yaw, pitch, roll
        "%.2f,%.2f,%.2f," // ax, ay, az
        "%d",             // flight state
        t,
        lat, lon,
        gpsAlt, fix,
        barAlt, press, temp,
        yaw, pitch, roll,
        ax, ay, az,
        state
    );
}
