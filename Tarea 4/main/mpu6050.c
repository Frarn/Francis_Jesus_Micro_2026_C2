#include "mpu6050.h"
#include "i2c_master.h"

#define MPU6050_ADDR       0x68

#define PWR_MGMT_1         0x6B

#define ACCEL_XOUT_H       0x3B

void mpu6050_init(void)
{
    /*
        Despierta el sensor
    */

    i2c_write_register(
        MPU6050_ADDR,
        PWR_MGMT_1,
        0x00
    );
}

void mpu6050_read(mpu6050_data_t *d)
{
    uint8_t buffer[14];

    i2c_read_register(
        MPU6050_ADDR,
        ACCEL_XOUT_H,
        buffer,
        14
    );

    d->ax = (buffer[0]<<8)|buffer[1];
    d->ay = (buffer[2]<<8)|buffer[3];
    d->az = (buffer[4]<<8)|buffer[5];

    d->gx = (buffer[8]<<8)|buffer[9];
    d->gy = (buffer[10]<<8)|buffer[11];
    d->gz = (buffer[12]<<8)|buffer[13];
}

tilt_t mpu6050_get_tilt(mpu6050_data_t *d)
{
    const int limit = 8000;

    if(d->ax > limit)
        return TILT_RIGHT;

    if(d->ax < -limit)
        return TILT_LEFT;

    if(d->ay > limit)
        return TILT_FORWARD;

    if(d->ay < -limit)
        return TILT_BACK;

    return TILT_CENTER;
}

const char *tilt_to_string(tilt_t t)
{
    switch(t)
    {
        case TILT_LEFT:
            return "LEFT";

        case TILT_RIGHT:
            return "RIGHT";

        case TILT_FORWARD:
            return "FORWARD";

        case TILT_BACK:
            return "BACK";

        default:
            return "CENTER";
    }
}