// sdcard.c — C-STYLE PSEUDOCODE


void sd_init(void) {
    // Setup SPI hardware
    // Setup MISO, MOSI, SCK pins
    // Set CS HIGH
    // Mount FAT filesystem
}

void sd_open_file(char *filename) {
    // Open "filename" in append mode
    // Create if it doesn't exist
}

void sd_write_line(char *text) {
    // Write text string to the file
    // Write newline character
}

void sd_close_file(void) {
    // Close file handle
}
