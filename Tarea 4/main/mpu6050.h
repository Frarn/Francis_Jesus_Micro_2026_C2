#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;

    int16_t gx;
    int16_t gy;
    int16_t gz;

} mpu6050_data_t;

typedef enum
{
    TILT_CENTER = 0,
    TILT_LEFT,
    TILT_RIGHT,
    TILT_FORWARD,
    TILT_BACK

} tilt_t;

void mpu6050_init(void);

void mpu6050_read(mpu6050_data_t *data);

tilt_t mpu6050_get_tilt(mpu6050_data_t *data);

const char *tilt_to_string(tilt_t tilt);

#endif