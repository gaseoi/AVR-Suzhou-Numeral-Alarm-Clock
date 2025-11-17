#include <AVR/io.h>
#include <util/delay.h> 
#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include "i2c.h"
#include "ssd1306.h"
#include "mpu6050.h"
#include "ds1307.h"
#include "USART.h"

#define TRIGGERTIME 1347 // replace with your desired alarm time
#define AXNOALARMTHRES 5000
#define AXALARMTHRES -5000

ISR(WDT_vect){
  //Nothing
}

void enableWatchDogInterrupt(void){
  cli();
  wdt_reset();
  MCUSR &= ~_BV(WDRF); 
  WDTCSR |= _BV(WDCE) | _BV(WDE);
  WDTCSR = _BV(WDIE) | _BV(WDP0) |  _BV(WDP3);
  sei();
}

void enterPowerDown(void){
  SMCR |= _BV(SM1);
  SMCR |= _BV(SE);
  sleep_mode();
}

void initADC(void){
  ADMUX = (_BV(REFS0)|_BV(MUX1)|_BV(MUX0)|_BV(ADLAR)); //Use AVCC, and connect PC3 to ADC
  ADCSRA = (_BV(ADEN)|_BV(ADPS2)|_BV(ADPS0)); // Set ADC enable bit, and set 32 division factor
  DIDR0 = _BV(ADC3D); //Disables digital input buffer
}

uint8_t readADC(void){
  ADCSRA |= _BV(ADSC); //starts a conversion
  loop_until_bit_is_clear(ADCSRA, ADSC); // Wait for conversion 
  return ADCH;
}

//Reads ADC value and maps readout to a previously defined value for battery percentage
//Algorythm involves finding the bat percentage ADC value closest to the ADC readout
//Also take running average to improve accuracy
uint8_t returnBatPercent(void){
  uint8_t batADCVals[6] = {197, 203, 209, 215, 222, 227};
  uint8_t percentArray[6] = {20, 40, 60, 80, 90, 100};
  uint8_t desiredBatIndex = 0;
  uint16_t sumBatADCVal = 0;
  printString("singlereads:");
  for (uint8_t i=0;i<5;i++){
    printByte(readADC());
    printString(", ");
    sumBatADCVal += readADC();
  }
  printString("\r\n");
  uint8_t actualBatADCVal = sumBatADCVal / 5;
  
  uint8_t smallestDiff = 0xFF;
  for (uint8_t i=0; i<6; i++){
    uint8_t diffCurrent;
    if (batADCVals[i]>actualBatADCVal){
      diffCurrent = batADCVals[i] - actualBatADCVal;
    }
    else{
      diffCurrent = actualBatADCVal - batADCVals[i];
    }
    if (diffCurrent <= smallestDiff) {
      smallestDiff = diffCurrent;
      desiredBatIndex = i;
    }
  }
  return percentArray[desiredBatIndex];
}

void initPWM(void){
  TCCR0A |= (_BV(WGM00)|_BV(WGM01)); // Fast PWM
  TCCR0A |= (_BV(COM0A1)); //Non inverted
  TCCR0B |= (_BV(CS01)); //8 prescaler ~4kHz in this case
}

void stopPWM(void){
  TCCR0A &= ~(_BV(COM0A1)|_BV(COM0A0)); //Disconnects 0C0A
  PORTD &= ~(_BV(PD6));
}

uint16_t returnAvgAx(void){
  int16_t avgax = 0;
  int32_t sumax = 0;
  for (uint8_t i=0; i<5; i++){
    int16_t ax, ay, az, gx, gy, gz;
    readMPU(&ax, &ay, &az, &gx, &gy, &gz);
    printInt16(ax);
    printString(", ");
    sumax += ax;
  }
  return avgax = sumax/5;
}

int main(void){
  clock_prescale_set(clock_div_1);
  initI2C();
  initSSD();
  clearSSD();
  initMPU();
  initUSART();
  drawAlarmBasic();
  drawColon();
  drawBatteryShell();
  initADC();
  DDRD |= _BV(PD6);
  while(1){
    int16_t averageax = returnAvgAx();

    if (averageax < AXNOALARMTHRES && averageax > AXALARMTHRES){ // IF IMU isnt at desired orientation, switch screen off
      displayOFF();
    }
    else{
      uint8_t tenhour, hour, tenmin, min;
      readclock(&tenhour, &hour, &tenmin, &min);
      drawTime(tenhour, hour, tenmin, min);

      uint8_t currentBat = returnBatPercent();
      if (currentBat > 20) {
        drawBatteryCentre(returnBatPercent());

      }
      else{
        drawBatteryAlert();
      }

      printString("Time: ");
      transmitByte('0' + tenhour);
      transmitByte('0' + hour);
      printString(":");
      transmitByte('0' + tenmin);
      transmitByte('0' + min);
      printString("\r\n");

      printString("Actual ADC readout: ");
      printByte(returnBatPercent());
      printString("\r\n");

      printString("Power percentage: ");
      printByte(readADC());
      printString("\r\n");

      displayON();

      if (averageax > AXNOALARMTHRES){
        flipDisplay();
        drawAlarmInactive();
       }
     
      else if (averageax < AXALARMTHRES){
        nonflipDisplay();
        drawAlarmActive();
        uint16_t concatenatedTime = (uint16_t)tenhour*1000 + (uint16_t)hour*100 + (uint16_t)tenmin*10 + (uint16_t)min;
        if (concatenatedTime == TRIGGERTIME){
          initPWM();
          uint8_t secTraversed = 0;
          while (secTraversed < 255 && averageax < AXALARMTHRES){
            readclock(&tenhour, &hour, &tenmin, &min);
            drawTime(tenhour, hour, tenmin, min);
            OCR0A = 80;
            _delay_ms(500);
            OCR0A = 160;
            _delay_ms(500);
            averageax = returnAvgAx();
            secTraversed ++;
          }
          stopPWM();
          flipDisplay();
          drawAlarmInactive();
        }
       }
    }
    _delay_ms(1000); 
    enableWatchDogInterrupt();
    enterPowerDown();

  }
  return 0;
}