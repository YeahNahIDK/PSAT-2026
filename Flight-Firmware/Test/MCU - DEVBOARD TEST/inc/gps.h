/*
 * ============================================================================
 * GPS DRIVER - Header File
 * ============================================================================
 * 
 * Driver for CD-PA1616D GPS module via UART
 * 
 * Configuration for MSP430FR2433:
 * - GPS on UCA1 (UART1)
 * - P1.6 = UCA1TXD (MSP → GPS RX)
 * - P1.7 = UCA1RXD (GPS TX → MSP)
 * 
 * The GPS sends NMEA sentences like:
 * $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
 * 
 * We parse these to extract:
 * - Latitude, Longitude
 * - Number of satellites
 * - Fix quality
 */

#ifndef GPS_H
#define GPS_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// CONSTANTS
// ============================================================================

#define GPS_BUFFER_SIZE     128     // Max NMEA sentence length (~82 typical)
#define GPS_BAUD_RATE       9600    // Standard GPS baud rate

// ============================================================================
// GPS DATA STRUCTURE
// ============================================================================

typedef struct {
    float latitude;          // Latitude in decimal degrees (e.g., -37.7833)
    float longitude;         // Longitude in decimal degrees (e.g., 175.2667)
    uint8_t satellites;      // Number of satellites in use
    uint8_t fix_quality;     // Fix quality: 0=none, 1=GPS, 2=DGPS
    float altitude;          // Altitude in meters above sea level
    float speed_knots;       // Speed over ground in knots
    bool valid;              // True if we have a valid GPS fix
} gps_data_t;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

/**
 * Initialize GPS UART.
 * 
 * Configures:
 * - UCA1 (UART1) for GPS communication
 * - 9600 baud rate
 * - 8N1 (8 data bits, no parity, 1 stop bit)
 * - RX interrupt enabled
 */
void gps_init(void);

/**
 * Read GPS data (non-blocking).
 * 
 * Call this frequently in your main loop.
 * It processes any received NMEA sentences and updates internal GPS data.
 * 
 * Returns:
 *   true  - New GPS data available
 *   false - No new data since last call
 * 
 * Example:
 *   if (gps_read()) {
 *       // GPS data was updated
 *   }
 */
bool gps_read(void);

/**
 * Get latest GPS data.
 * 
 * Parameters:
 *   data - Pointer to structure to fill with GPS data
 * 
 * Returns:
 *   true  - Data is valid (GPS has a fix)
 *   false - Data is invalid (no GPS fix)
 * 
 * Example:
 *   gps_data_t position;
 *   if (gps_get_data(&position)) {
 *       // position.latitude, position.longitude are valid
 *   }
 */
bool gps_get_data(gps_data_t* data);

/**
 * Get raw NMEA sentence (for debugging).
 * 
 * Parameters:
 *   buffer      - Buffer to store NMEA sentence
 *   buffer_size - Size of buffer
 * 
 * Returns:
 *   true  - Sentence copied to buffer
 *   false - No complete sentence available
 * 
 * Example:
 *   char nmea[GPS_BUFFER_SIZE];
 *   if (gps_get_raw_nmea(nmea, sizeof(nmea))) {
 *       // nmea contains something like "$GPGGA,123519,..."
 *   }
 */
bool gps_get_raw_nmea(char* buffer, uint8_t buffer_size);

/**
 * Put GPS module to sleep (low power mode).
 * 
 * Note: CD-PA1616D supports standby mode.
 * Send specific commands to enable/disable.
 */
void gps_sleep(void);

/**
 * Wake GPS module from sleep.
 */
void gps_wake(void);

#endif // GPS_H
