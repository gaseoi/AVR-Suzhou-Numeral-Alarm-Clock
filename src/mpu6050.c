#include "mpu6050.h"
#include "i2c.h"
#include <avr/io.h>

void initMPU(void) {
    //write to the power management 1 register
    i2cStart();
    i2cSend(MPU_WRITE_ADDR);
    i2cSend(PWR_MGMT_1);
    i2cSend(0b00000000);
    i2cStop();

    //write to sample rate divider register
    i2cStart();
    i2cSend(MPU_WRITE_ADDR);
    i2cSend(SMPRT_DIV);
    i2cSend(0x04);
    i2cStop();
    //configure accelerometer
    i2cStart();
    i2cSend(MPU_WRITE_ADDR);
    i2cSend(ACCEL_CONFIG);
    i2cSend(0x08);
    i2cStop();
    //configure gyro
    i2cStart();
    i2cSend(MPU_WRITE_ADDR);
    i2cSend(GYRO_CONFIG);
    i2cSend(0x08);
    i2cStop();
}

void readMPU(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz){
    //extract data from accelerometer + gyro measurement registers
    i2cStart();
    i2cSend(MPU_WRITE_ADDR);
    i2cSend(ACCEL_XOUT_H);
    i2cStart();
    i2cSend(MPU_READ_ADDR);
    uint8_t buffer[14];
    for (int i = 0; i < 13; i++){
        buffer[i] = i2cReadAck();
    }
    buffer[13] = i2cReadNoAck();
    i2cStop();
    *ax = (int16_t)(((uint16_t)buffer[0]) << 8) | buffer[1];
    *ay = (int16_t)(((uint16_t)buffer[2]) << 8) | buffer[3];
    *az = (int16_t)(((uint16_t)buffer[4]) << 8) | buffer[5];
    *gx = (int16_t)(((uint16_t)buffer[8]) << 8) | buffer[9];
    *gy = (int16_t)(((uint16_t)buffer[10]) << 8) | buffer[11];
    *gz = (int16_t)(((uint16_t)buffer[12]) << 8) | buffer[13];
}