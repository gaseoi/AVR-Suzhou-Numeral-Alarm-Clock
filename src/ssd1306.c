#include "ssd1306.h"
#include "i2c.h"
#include <avr/io.h>

void initSSD(void){
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x00); //Co and DC bit both 0. 

    i2cSend(DISPLAYOFF); 

    //set mux ratio
    i2cSend(MULTIPLEXRATIOSET);
    i2cSend(MULTIPLEXVAL);

    //set display offset
    i2cSend(DISPLAYOFFSETSET);
    i2cSend(0x00);

    //Set display start line
    i2cSend(DISPLAYSTARTLINE);

    //Set display mode (breaking order from datasheet)
    i2cSend(ADDRMODESET);
    i2cSend(VERTICAL);

    //set segment remap
    i2cSend(SEGREMAPNORMAL);

    //set COM output scan direction
    i2cSend(NORMALCOMSCANDIR);

    //Set COM pins hardware config
    i2cSend(COMPINCONFSET);
    i2cSend(COMPINCONF); 

    //Set contrast control
    i2cSend(CONTRASTSET);
    i2cSend(CONTRASTVAL);

    //Disable entire display on
    i2cSend(RESUMETORAM);

    //Set normal display
    i2cSend(NORMALDISPLAY);

    //Set osc freq
    i2cSend(DISPLAYCLOCKSET);
    i2cSend(DISPLAYCLOCKFREQ);

    //Enable charge pump regulator
    i2cSend(CHARGEPUMPSET);
    i2cSend(CHARGEPUMPON);

    //entire display on
    i2cSend(DISPLAYON);
    i2cStop();
}

void clearSSD(void){
    //Send the command for write locations, then make a seperate
    //i2c send for the data
    positionCommand(0,127,0,7);

    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    for (uint16_t i = 0; i<1024; i++){
        i2cSend(0x00);
    }
    i2cStop();
}

void drawColon(void){
    positionCommand(42,45,2,3);

    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    for (uint8_t i = 0; i<4; i++){
        i2cSend(0b00001111);
        i2cSend(0b11110000);
    }
    i2cStop();
}

//Converts the bool array to uint*_t. Note that order gets reversed again here, so
//if boolArray is 01234567, finalbyte becomes 76543210
uint8_t boolArrayToByte(bool *boolArray){
    uint8_t finalByte = 0;
    for (uint8_t i = 0; i<8; i++){
        finalByte <<=1;
        if (boolArray[i] == 1){
            finalByte |= 1;
        }
    }
    return finalByte;
}

//Takes in a byte and i2cSends the same byte but 'stretched' over 2 bytes
//Note that for given an example condensedByte 01234567, this will send 77665544, then 33221100
void decondenseMapSend(uint8_t condensedByte){
    uint8_t splitByte[2];
    splitByte[0] = (0b11110000 & condensedByte) >> 4;
    splitByte[1] = (0b00001111 & condensedByte);
    for (uint8_t i=0; i<2; i++){ // Loops twice to send 2 bytes, one for the first nibble and one for the second nibble.
        bool buffer[8];
        for (uint8_t j=0; j<4; j++){ //Starting from index 0 stepping up 2 at a time ending at index 7
            buffer[2*j] = ((splitByte[i]>>(j))&(0b00000001));
            buffer[2*j + 1] = ((splitByte[i]>>(j))&(0b00000001));
        }
        i2cSend(boolArrayToByte(buffer));
    }
}

//Takes a number, copies over a local copy of the condensed number bitmap
//Then transmits 2 bytes, then repeat transmitting the same 2 bytes, at a time
//effectively stretching the byte over the x axis.
void drawNum(uint8_t num){
    uint8_t condensedBuffer[16];
    memcpy_P(condensedBuffer, CONDENSEDMAP[num],16);

    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    
    for (uint8_t uniqueCol = 0; uniqueCol <8; uniqueCol++){
        for (uint8_t count = 0; count<2; count++){
            decondenseMapSend(condensedBuffer[2*uniqueCol]);
            decondenseMapSend(condensedBuffer[2*uniqueCol + 1]);
        }
    }

    i2cStop();

}

//Sends the column and page range specific to each digit for the clock numbers
void setColnPgAddr(uint8_t colstart){
    positionCommand(colstart,colstart+15,1,4);
}

//Takes the time and displays it on the display
void drawTime(uint8_t tenhour, uint8_t hour, uint8_t tenmin, uint8_t min){
    setColnPgAddr(8);
    drawNum(tenhour);
    setColnPgAddr(24);
    drawNum(hour);
    setColnPgAddr(48);
    drawNum(tenmin);
    setColnPgAddr(64);
    drawNum(min);
}

//Template function which sends the i2c command to set draw position
void positionCommand(uint8_t startcol, uint8_t endcol, uint8_t startpg, uint8_t endpg){
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x00); //Co 0 and D/C 0
    i2cSend(SETCOLADDR);
    i2cSend(startcol);
    i2cSend(endcol);
    i2cSend(SETPAGEADDR);
    i2cSend(startpg);
    i2cSend(endpg);
    i2cStop();
}

//Draws the enclosure surrounding the battery
void drawBatteryShell(void){
    positionCommand(94,94,1,5);
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    i2cSend(0b11000000);
    for (uint8_t i = 0; i<3; i++){
        i2cSend(0xFF);
    }
    i2cSend(0b00000011);
    i2cStop();

    positionCommand(114,114,1,5);
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    i2cSend(0b11000000);
    for (uint8_t i = 0; i<3; i++){
        i2cSend(0xFF);
    }
    i2cSend(0b00000011);
    i2cStop();

    positionCommand(95,113,1,1);
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    for (uint8_t i = 0; i<6; i++){
        i2cSend(0b01000000);
    }
    for (uint8_t i = 0; i<7; i++){
        i2cSend(0b01111000);
    }
    for (uint8_t i = 0; i<6; i++){
        i2cSend(0b01000000);
    }
    i2cStop();

    positionCommand(95,113,5,5);
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    for (uint8_t i = 0; i<19; i++){
        i2cSend(0b00000010);
    }
    i2cStop();
}

//Sends command bytes prior to writing to the centre of the battery
void batteryCentreCommand(void){
    positionCommand(96,112,2,4);
}

//Replaces battery centre with an exclaimation mark
void drawBatteryAlert(void){
    batteryCentreCommand();
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    for (uint8_t i=0; i<7; i++){
        i2cSend(0x00);
        i2cSend(0x00);
        i2cSend(0x00);
    }
    for (uint8_t i=0; i<3; i++){
        i2cSend(0b11111100);
        i2cSend(0b11111111);
        i2cSend(0b00111000);
    }
    for (uint8_t i=0; i<7; i++){
        i2cSend(0x00);
        i2cSend(0x00);
        i2cSend(0x00);
    }
    i2cStop();
}

//Given a battery percentage, converts the percentage into a visual representation 
//of battery fullness on the OLED
void drawBatteryCentre(uint8_t percentage){
    uint8_t numofpixelslit = (uint8_t)((percentage/100.0)*24); // Calculates number of vertical pixels needing to be lit
    bool boolBuffers[3][8] = {}; // Makes 3 bool buffers each holding 8 bools, then populate them based on the numofpixelslit
    for (uint8_t i=0; i< numofpixelslit; i++){ 
        if (i<8){
            boolBuffers[0][i] = 1;
        }
        else if (i<16){
            uint8_t j = i - 8;
            boolBuffers[1][j] = 1;
        }
        else {
            uint8_t k = i - 16;
            boolBuffers[2][k] = 1;
        }
    }
    
    batteryCentreCommand();

    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    for (uint8_t i=0; i<16; i++){
        i2cSend(boolArrayToByte(boolBuffers[2]));
        i2cSend(boolArrayToByte(boolBuffers[1]));
        i2cSend(boolArrayToByte(boolBuffers[0]));
    }
    i2cStop();
}

//Displays the symbol for the alarm in its basic state
void drawAlarmBasic(void){
    uint8_t buff[5];
    memcpy_P(buff, ALARMMAP[0],5);
    positionCommand(98,102,6,6);
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    for (uint8_t i=0; i<5; i++){
        i2cSend(buff[i]);
    }
    i2cStop();
}

void drawAlarmInactive(void){
    uint8_t buff[5];
    memcpy_P(buff, ALARMMAP[1],5);
    positionCommand(104,108,6,6);
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    for (uint8_t i=0; i<5; i++){
        i2cSend(buff[i]);
    }
    i2cStop();
}

void drawAlarmActive(void){
    uint8_t buff[5];
    memcpy_P(buff, ALARMMAP[2],5);
    positionCommand(104,108,6,6);
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x40);// Co 0 and D/C 1
    for (uint8_t i=0; i<5; i++){
        i2cSend(buff[i]);
    }
    i2cStop();
}

//Flips the screen along the x and y axis
void flipDisplay(void){
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x00);// Co 0 and D/C 0 
    i2cSend(FLIPPEDCOMSCANDIR);
    i2cSend(SEGREMAPINVERT);
    i2cStop();
}

//Reverts the display to normal orientation
void nonflipDisplay(void){
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x00);// Co 0 and D/C 0 
    i2cSend(NORMALCOMSCANDIR);
    i2cSend(SEGREMAPNORMAL);
    i2cStop();
}

//Turns display off
void displayOFF(void){
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x00);// Co 0 and D/C 0 
    i2cSend(DISPLAYOFF);
    i2cStop();
}

//Turns display back on
void displayON(void){
    i2cStart();
    i2cSend(SSD_WRITEADDR);
    i2cSend(0x00);// Co 0 and D/C 0 
    i2cSend(DISPLAYON);
    i2cStop();
}