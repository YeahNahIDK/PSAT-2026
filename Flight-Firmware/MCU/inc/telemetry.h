// telemetry.c - BUILD TELEMETRY PAYLOAD
//
// Gather data from GPS + state machine and turn it into a single string.
//
// Example output:
//      "LAT:-36.8523,LON:174.7633,ALT:143.2,SATS:7,STATE:ASC"

void telemtry_generate(char *buffer){

    // Read values from GPS:
    //      float  lat = gps_lat();
    //      float  lon = gps_lon();
    //      float  alt = gps_alt();
    //      int   sats = gps_sats();

    // Read flight state:
    //      FlightState s = state_get();

    // Convert state enum into text (IDLE/ASC/APO/DEX/LANDED)

    // Use sprintf-style formatting to build final string

    // Write result into user-provided buffer??? i think lol
}