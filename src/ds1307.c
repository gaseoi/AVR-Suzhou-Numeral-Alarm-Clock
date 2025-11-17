#include <avr/io.h>
#include "i2c.h"
#include "ds1307.h"

void calibrateclock(uint16_t fourdigit) {
    /*Expects four digit representation of time i.e. 12:30pm is 1230
    Puts each digit in timebuf, then passes it into timekeeper registers*/
    uint8_t timebuf[4];
    for (int counter = 3; counter >= 0; counter--) {
        timebuf[counter] = fourdigit%10;
        fourdigit/=10;
    }
    /* Ensure seconds register has clock-halt bit cleared (start the clock)
       write seconds=0 first, then write minutes+hours starting at MINUTEREG (0x01) */
    i2cStart();
    i2cSend(DS_WRITE_ADDR);
    i2cSend(0x00);            /* seconds register */
    i2cSend(0x00);            /* seconds = 0, CH bit = 0 -> start clock */
    i2cStop();

    i2cStart();
    i2cSend(DS_WRITE_ADDR);
    i2cSend(MINUTEREG);
    i2cSend((0<<7)|(timebuf[2]<<4)|timebuf[3]);
    i2cSend((00<<6)|(timebuf[0]<<4)|timebuf[1]);
    i2cStop();
}

void readclock(uint8_t* tenhour, uint8_t* hour, uint8_t* tenmin, uint8_t* min){
    uint8_t regbuf[2];
    i2cStart();
    i2cSend(DS_WRITE_ADDR);
    i2cSend(MINUTEREG);
    i2cStop();

    i2cStart();
    i2cSend(DS_READ_ADDR);
    regbuf[0] = i2cReadAck();
    regbuf[1] = i2cReadNoAck();
    i2cStop();
    *min = (regbuf[0] & 0b00001111);
    *tenmin = (regbuf[0] >> 4) & 0b00000111;
    *hour = (regbuf[1] & 0b00001111);
    *tenhour = (regbuf[1] >> 4) & 0b00000011 ; 
}