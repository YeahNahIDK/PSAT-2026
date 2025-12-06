// sdcard.h — C-STYLE PSEUDOCODE


#ifndef SDCARD_H
#define SDCARD_H

void sd_init(void);                     // initialise SPI + filesystem
void sd_open_file(char *filename);      // open/create a file
void sd_write_line(char *text);         // append a line
void sd_close_file(void);               // close file safely

#endif
