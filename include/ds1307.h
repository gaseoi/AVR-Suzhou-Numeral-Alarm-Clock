#include <avr/io.h>

#define DS_READ_ADDR 0b11010001
#define DS_WRITE_ADDR 0b11010000
#define MINUTEREG 0x01

void calibrateclock(uint16_t fourdigit);
void readclock(uint8_t* tenhour, uint8_t* hour, uint8_t* tenmin, uint8_t* min);