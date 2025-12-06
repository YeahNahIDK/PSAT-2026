// gps.c — IMPLEMENTATION 

// Keeps a buffer that fills until '\n'. When a full sentence arrives,
// we parse it and extract useful information.
// Only GGA and RMC sentences matter for flight telemetry.

static char nmea_buffer[120];
static int buffer_index = 0;

static float last_lat = 0;
static float last_lon = 0;
static float last_alt = 0;
static int   fix_status = 0;

void gps_init() {
    // Configure UART #1 for 9600 8N1
    // Reset buffer_index
}

void gps_read() {
    // while (UART_has_data):
    //     char c = UART_read_byte();
    //
    //     if (c is not newline):
    //         append to nmea_buffer
    //     else:
    //         we have a full sentence → parse it
    //         reset buffer_index
}

static void parse_sentence() {
    // Check if sentence starts with "$GPGGA" or "$GPRMC".
    // Split by commas.
    // Convert longitude/latitude from NMEA format (DDMM.MMMM to degrees).
    // Save values to last_lat, last_lon, last_alt.
    // Update fix_status.
}

int gps_has_fix()  { return fix_status; }
float gps_lat()    { return last_lat; }
float gps_lon()    { return last_lon; }
float gps_alt()    { return last_alt; }
