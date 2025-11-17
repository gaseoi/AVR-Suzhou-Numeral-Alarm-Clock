#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

//from fundimental command table pg28
#define SSD_WRITEADDR 0b01111000
#define CONTRASTSET 0x81
#define CONTRASTVAL 0x7F //Select default contrast
#define RESUMETORAM 0xA4 //Entire display on. Output RAM content
#define NORMALDISPLAY 0xA6
#define DISPLAYON 0xAF
#define DISPLAYOFF 0xAE

//from addressing setting table pg30
#define ADDRMODESET 0x20
#define VERTICAL 0b01 //vertical addressing mode
//For use after initialisation:
#define SETCOLADDR 0x21
#define SETPAGEADDR 0x22

//from hardware config table pg31
#define DISPLAYSTARTLINE 0x40
#define SEGREMAPINVERT 0xA0
#define SEGREMAPNORMAL 0xA1
#define MULTIPLEXRATIOSET 0xA8
#define MULTIPLEXVAL 63 //sets 64 MUX
#define NORMALCOMSCANDIR 0xC8
#define FLIPPEDCOMSCANDIR 0xC0
#define DISPLAYOFFSETSET 0xD3
#define COMPINCONFSET 0xDA
#define COMPINCONF 0b00010010 //Sequential COM pins, enable COM L/R remap 

//from timing & driving setting table pg32
#define DISPLAYCLOCKSET 0xD5
#define DISPLAYCLOCKFREQ 0x80 //default
#define PRECHARGESET 0xD9
#define VCOMHDESELECT 0xDB

//from charge bump settings pg3 application notes
#define CHARGEPUMPSET 0x8D
#define CHARGEPUMPON 0x14

static const PROGMEM uint8_t CONDENSEDMAP[][16] = {
    {
        0b00000000, 0b00000000,
        0b00000011, 0b11000000,
        0b00000110, 0b01100000,
        0b00001100, 0b00110000,
        0b00001000, 0b00010000,
        0b00001100, 0b00110000,
        0b00000110, 0b01100000,
        0b00000011, 0b11000000

    }, //0
    {
        0b00000000, 0b00000000,
        0b00000000, 0b00000000,
        0b00000000, 0b00000000,
        0b00000000, 0b00000000,
        0b00111111, 0b11111100,
        0b00000000, 0b00000000,
        0b00000000, 0b00000000,
        0b00000000, 0b00000000
    }, //1

    {
        0b00000000, 0b00000000,
        0b00000000, 0b00000000,
        0b00001111, 0b11000000,
        0b00000000, 0b00000000,
        0b00000000, 0b00000000,
        0b00111111, 0b11110000,
        0b00000000, 0b00000000,
        0b00000000, 0b00000000
    }, //2

    {
        0b00000000, 0b00000000,
        0b00000000, 0b00000000,
        0b00111111, 0b11110000,
        0b00000000, 0b00000000,
        0b00011111, 0b11100000,
        0b00000000, 0b00000000,
        0b00111111, 0b11111100,
        0b00000000, 0b00000000
    }, //3

    {
        0b00000000, 0b00000000,
        0b00001000, 0b00010000,
        0b00001100, 0b00100000,
        0b00000110, 0b01000000,
        0b00000001, 0b10000000,
        0b00000111, 0b01000000,
        0b00011000, 0b00100000,
        0b00000000, 0b00000000
    }, //4

    {
        0b00000000, 0b00000000,
        0b00000100, 0b00100000,
        0b00001110, 0b11010000,
        0b00010001, 0b00001000,
        0b00100011, 0b00001000,
        0b00000100, 0b10010000,
        0b00000000, 0b01100000,
        0b00000000, 0b00000000
    }, //5

    {
        0b00000000, 0b00000000,
        0b00000000, 0b00000000,
        0b00000000, 0b01000000,
        0b00000000, 0b01000000,
        0b00001111, 0b11000000,
        0b00000000, 0b01000000,
        0b00000000, 0b01000000,
        0b00000000, 0b00000000
    }, //6

    {
        0b00000000, 0b00000000,
        0b00000000, 0b00000000,
        0b00000000, 0b10100000,
        0b00000000, 0b10100000,
        0b00001111, 0b10100000,
        0b00000000, 0b10100000,
        0b00000000, 0b10000000,
        0b00000000, 0b00000000
    }, //7

    {
        0b00000000, 0b00000000,
        0b00000000, 0b00000000,
        0b00000001, 0b01010000,
        0b00000001, 0b01010000,
        0b00000111, 0b01010000,
        0b00000001, 0b01010000,
        0b00000001, 0b00010000,
        0b00000000, 0b00000000
    }, //8

    {
        0b00000000, 0b00000000,
        0b00000101, 0b00011000,
        0b00001100, 0b10100000,
        0b00110100, 0b01000000,
        0b00000101, 0b10100000,
        0b00000110, 0b00010000,
        0b00000100, 0b00001000,
        0b00000000, 0b00000000
    }, //9

};

static const PROGMEM uint8_t ALARMMAP[][5] = {
    {
        0b00011100,
        0b00011100,
        0b00111110,
        0b01111111,
        0b00000000
    }, //Speaker symbol

    {
        0b00100010,
        0b00010100,
        0b00001000,
        0b00010100,
        0b00100010
    }, //Cross symbol

    {
        0b00100010,
        0b00011100,
        0b01000001,
        0b00111110,
        0b00000000
    } //Speaking symbol

};

void initSSD(void);
void clearSSD(void);
void drawColon(void);
uint8_t boolArrayToByte(bool *boolArray);
void decondenseMapSend(uint8_t condensedByte);
void drawNum(uint8_t num);
void setColnPgAddr(uint8_t colstart);
void drawTime(uint8_t tenhour, uint8_t hour, uint8_t tenmin, uint8_t min);
void positionCommand(uint8_t startcol, uint8_t endcol, uint8_t startpg, uint8_t endpg);
void drawBatteryShell(void);
void drawBatteryCentre(uint8_t percentage);
void drawAlarmBasic(void);
void drawAlarmInactive(void);
void drawAlarmActive(void);
void drawBatteryAlert(void);
void flipDisplay(void);
void nonflipDisplay(void);
void displayOFF(void);
void displayON(void);