# AVR-Suzhou-Numeral-Alarm-Clock
## Intro
This is a personal project applying bare-metal C concepts from the "Make AVR Programming" book by Elliot Williams to create an IMU integrated alarm clock displaying Suzhou numerals on OLED. All drivers written by me in C by reading datasheets and doing bitwise manipulations to write to individual registers. 

> The Suzhou numerals, also known as Sūzhōu mǎzi (蘇州碼子), is a numeral system used in China before the introduction of Hindu-Arabic numerals.

From Suzhou numerals Wikipedia page

Could I have made this overnight using the comfort of existing arduino libraries? Yes, but I wanted to get away from the arduino framework to work with hardware at a lower level. 

## Key features:

+ Project consists of an ATMEGA168p communicating via I2C to an MPU6050 6 axis IMU, DS1307 RTC, and SSD1306 OLED.
+ PWM is used to control a small haptic motor through the base of a transistor. 
+ The IMU is used for tilt detection so that clock mode is selected by changing its orientation.
  + There are three modes: Screen-off, screen-on (alarm on), and screen on (alarm off).
+ The on-board ADC is used for monitering battery power level with a voltage divider.
+ The screen not only displays the time using Suzhou numerals, but also displays the current clock mode, and battery percentage graphically.
+ The ATMEGA168p is running at 8MHz, configured to enter power-down every ~8.0s between each wakeup via the watchdog timer enabled as an interrupt.
  + This is done to minimize power consumption as the clock really doesnt need constant updates every second/millisecond. 

## Setup Guide:
Pin mapping summarized below:
| ATMEGA168 pin number  | Destination |
| ------------- | ------------- |
| PD6 | PWM to 2N2222 NPN transistor base for controlling haptic motor |
| PC5  | I2C SCL, pulled up to 5V via 2K Ohm resistor  |
| PC4  | I2C SDA, pulled up to 5V via 2K Ohm resistor |
| PC3  | ADC for monitering battery life |

I used the PlatformIO VSCode extension and AVRDUDESS as part of my AVR programming toolchain. For AVRDUDESS I bought a cheap USBasp programmer and FT232 USB to Serial adaptor for debugging.

## Potential future features:
+ SPI SD card reader for storing and changing wakeup times. 
