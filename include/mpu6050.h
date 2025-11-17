#include <avr/io.h>

#define MPU_READ_ADDR 0b11010011
#define MPU_WRITE_ADDR 0b11010010
#define WHOAMI_REGISTER 0x75
#define PWR_MGMT_1 0x6B
#define SMPRT_DIV 0x19
#define ACCEL_CONFIG 0x1C
#define GYRO_CONFIG 0x1B
#define ACCEL_XOUT_H 0x3B

void initMPU(void);

void readMPU(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);

