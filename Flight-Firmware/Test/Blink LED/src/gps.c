/*
 * ============================================================================
 * GPS DRIVER - Implementation (Using UART Manager)
 * ============================================================================
 * 
 * Driver for CD-PA1616D GPS module
 * Uses UART manager for all communication
 */

#include "./inc/gps.h"
#include "./inc/uart_manager.h"
#include <string.h>
#include <stdlib.h>

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

// Current NMEA sentence being assembled
static char nmea_sentence[GPS_BUFFER_SIZE];
static uint8_t nmea_index = 0;

// Latest parsed GPS data
static gps_data_t current_gps_data = {0};
static bool data_updated = false;

// ============================================================================
// PRIVATE FUNCTION PROTOTYPES
// ============================================================================

static bool parse_nmea_sentence(const char* sentence);
static bool parse_gpgga(const char* sentence);
static bool parse_gprmc(const char* sentence);
static float nmea_to_decimal(const char* nmea_coord, char direction);

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

void gps_init(void) {
    /*
     * Initialize GPS using UART manager
     * GPS uses UART_A1 at 9600 baud
     */
    uart_init(UART_A1, BAUD_9600);
    
    // Initialize state
    nmea_index = 0;
    data_updated = false;
    current_gps_data.valid = false;
}

bool gps_read(void) {
    /*
     * PROCESS RECEIVED GPS DATA
     * 
     * Reads from UART and assembles NMEA sentences.
     * When complete sentence received, parses it.
     * 
     * Returns true if new GPS data was parsed.
     */
    
    // Process all available bytes
    while (uart_available(UART_A1) > 0) {
        uint8_t c;
        if (!uart_read_byte(UART_A1, &c)) {
            break;
        }
        
        // NMEA sentences start with '$'
        if (c == '$') {
            nmea_index = 0;
            nmea_sentence[nmea_index++] = c;
        }
        // End of sentence is '\n' (or '\r')
        else if (c == '\n' || c == '\r') {
            if (nmea_index > 0) {
                nmea_sentence[nmea_index] = '\0';
                
                // Parse complete sentence
                data_updated = parse_nmea_sentence(nmea_sentence);
                nmea_index = 0;  // Reset for next sentence
                
                if (data_updated) {
                    return true;
                }
            }
        }
        // Build sentence
        else if (nmea_index > 0 && nmea_index < GPS_BUFFER_SIZE - 1) {
            nmea_sentence[nmea_index++] = c;
        }
        // Sentence too long - reset
        else if (nmea_index >= GPS_BUFFER_SIZE - 1) {
            nmea_index = 0;
        }
    }
    
    return false;
}

bool gps_get_data(gps_data_t* data) {
    if (data == NULL) {
        return false;
    }
    
    // Copy current GPS data
    *data = current_gps_data;
    
    return current_gps_data.valid;
}

bool gps_get_raw_nmea(char* buffer, uint8_t buffer_size) {
    if (buffer == NULL || buffer_size == 0) {
        return false;
    }
    
    if (nmea_index > 0 && nmea_index < buffer_size) {
        strncpy(buffer, nmea_sentence, buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
        return true;
    }
    
    return false;
}

void gps_sleep(void) {
    /*
     * CD-PA1616D Standby Mode
     * Send PMTK command to enter standby
     */
    const char* sleep_cmd = "$PMTK161,0*28\r\n";
    uart_write_string(UART_A1, sleep_cmd);
}

void gps_wake(void) {
    /*
     * Wake GPS by sending any character
     */
    uart_write_byte(UART_A1, 0xFF);
}

// ============================================================================
// PRIVATE FUNCTIONS - NMEA PARSING
// ============================================================================

static bool parse_nmea_sentence(const char* sentence) {
    /*
     * Parse NMEA sentence
     * Support GPGGA and GPRMC
     */
    
    if (strncmp(sentence, "$GPGGA", 6) == 0 || strncmp(sentence, "$GNGGA", 6) == 0) {
        return parse_gpgga(sentence);
    }
    else if (strncmp(sentence, "$GPRMC", 6) == 0 || strncmp(sentence, "$GNRMC", 6) == 0) {
        return parse_gprmc(sentence);
    }
    
    return false;
}

static bool parse_gpgga(const char* sentence) {
    /*
     * PARSE $GPGGA SENTENCE
     * 
     * Format:
     * $GPGGA,time,lat,N/S,lon,E/W,quality,sats,hdop,alt,M,...*checksum
     */
    
    char sentence_copy[GPS_BUFFER_SIZE];
    strncpy(sentence_copy, sentence, GPS_BUFFER_SIZE - 1);
    sentence_copy[GPS_BUFFER_SIZE - 1] = '\0';
    
    // Tokenize by comma
    char* token = strtok(sentence_copy, ",");
    uint8_t field = 0;
    
    char lat_str[16] = {0};
    char lat_dir = 'N';
    char lon_str[16] = {0};
    char lon_dir = 'E';
    
    while (token != NULL && field < 12) {
        switch (field) {
            case 2:  // Latitude
                strncpy(lat_str, token, sizeof(lat_str) - 1);
                break;
                
            case 3:  // N/S
                lat_dir = token[0];
                break;
                
            case 4:  // Longitude
                strncpy(lon_str, token, sizeof(lon_str) - 1);
                break;
                
            case 5:  // E/W
                lon_dir = token[0];
                break;
                
            case 6:  // Fix quality
                current_gps_data.fix_quality = atoi(token);
                current_gps_data.valid = (current_gps_data.fix_quality > 0);
                break;
                
            case 7:  // Number of satellites
                current_gps_data.satellites = atoi(token);
                break;
                
            case 9:  // Altitude
                current_gps_data.altitude = atof(token);
                break;
        }
        
        token = strtok(NULL, ",");
        field++;
    }
    
    // Convert coordinates if we have valid data
    if (strlen(lat_str) > 0 && strlen(lon_str) > 0) {
        current_gps_data.latitude = nmea_to_decimal(lat_str, lat_dir);
        current_gps_data.longitude = nmea_to_decimal(lon_str, lon_dir);
    }
    
    return current_gps_data.valid;
}

static bool parse_gprmc(const char* sentence) {
    /*
     * PARSE $GPRMC SENTENCE
     * 
     * Format:
     * $GPRMC,time,status,lat,N/S,lon,E/W,speed,course,date,...*checksum
     */
    
    char sentence_copy[GPS_BUFFER_SIZE];
    strncpy(sentence_copy, sentence, GPS_BUFFER_SIZE - 1);
    sentence_copy[GPS_BUFFER_SIZE - 1] = '\0';
    
    char* token = strtok(sentence_copy, ",");
    uint8_t field = 0;
    
    char lat_str[16] = {0};
    char lat_dir = 'N';
    char lon_str[16] = {0};
    char lon_dir = 'E';
    
    while (token != NULL && field < 10) {
        switch (field) {
            case 2:  // Status (A=valid, V=invalid)
                current_gps_data.valid = (token[0] == 'A');
                break;
                
            case 3:  // Latitude
                strncpy(lat_str, token, sizeof(lat_str) - 1);
                break;
                
            case 4:  // N/S
                lat_dir = token[0];
                break;
                
            case 5:  // Longitude
                strncpy(lon_str, token, sizeof(lon_str) - 1);
                break;
                
            case 6:  // E/W
                lon_dir = token[0];
                break;
                
            case 7:  // Speed in knots
                current_gps_data.speed_knots = atof(token);
                break;
        }
        
        token = strtok(NULL, ",");
        field++;
    }
    
    // Convert coordinates
    if (strlen(lat_str) > 0 && strlen(lon_str) > 0) {
        current_gps_data.latitude = nmea_to_decimal(lat_str, lat_dir);
        current_gps_data.longitude = nmea_to_decimal(lon_str, lon_dir);
    }
    
    return current_gps_data.valid;
}

static float nmea_to_decimal(const char* nmea_coord, char direction) {
    /*
     * Convert NMEA coordinate to decimal degrees
     * 
     * NMEA: ddmm.mmmm (lat) or dddmm.mmmm (lon)
     * Decimal: dd.dddddd
     */
    
    if (nmea_coord == NULL || strlen(nmea_coord) < 4) {
        return 0.0f;
    }
    
    // Find decimal point
    const char* dot = strchr(nmea_coord, '.');
    if (dot == NULL) {
        return 0.0f;
    }
    
    // Degrees length: 2 for lat, 3 for lon
    int deg_len = (direction == 'N' || direction == 'S') ? 2 : 3;
    
    // Extract degrees
    char degrees_str[4] = {0};
    strncpy(degrees_str, nmea_coord, deg_len);
    float degrees = atof(degrees_str);
    
    // Extract minutes
    float minutes = atof(nmea_coord + deg_len);
    
    // Convert to decimal
    float decimal = degrees + (minutes / 60.0f);
    
    // Apply direction (South and West are negative)
    if (direction == 'S' || direction == 'W') {
        decimal = -decimal;
    }
    
    return decimal;
}

/*
 * ============================================================================
 * USAGE EXAMPLE
 * ============================================================================
 * 
 * // In setup:
 * gps_init();
 * 
 * // In main loop:
 * if (gps_read()) {
 *     gps_data_t position;
 *     if (gps_get_data(&position)) {
 *         // position.latitude, position.longitude
 *         // position.satellites
 *         // position.altitude
 *         // position.valid == true
 *     }
 * }
 */
