// main.c 

// Main responsibilities:
//   - Initialise all hardware and sensors
//   - Read sensors continuously
//   - Update flight state machine
//   - Format telemetry string
//   - Send telemetry by LoRa
//   - Log telemetry to SD card


int main(void) {

  
    // HARDWARE INITIALISATION

    system_clock_init();
    system_uart_init();      // GPS, IMU, LoRa UARTs
    system_i2c_init();       // Barometer
    system_spi_init();       // SD card
    system_timer_init();     // millis()

    
    // SENSOR INITIALISATION

    gps_init();              // slow position data
    imu_init();              // orientation + accel
    baro_init();             // altitude source


    // OUTPUT DEVICE INITIALISATION
   
    lora_init();             // radio settings
    servo_init();            // sets PWM idle

    // SD CARD SETUP

    sd_init();
    sd_open_file("flight.txt");


    // FLIGHT STATE SETUP
    
    state_init();

    char buffer[256];


    // MAIN LOOP
 
    while (1) {

        // Update sensors
        gps_read();
        imu_read();
        baro_update();

        // Update flight state (detect apogee, start servo, etc.)
        state_update();

        // Format telemetry line
        telemetry_format(buffer);

        // Transmit to ground station
        lora_send(buffer);

        // Save to SD card
        sd_write_line(buffer);

        // Control loop frequency
        delay_ms(50);
    }

    sd_close_file();
    return 0;
}
