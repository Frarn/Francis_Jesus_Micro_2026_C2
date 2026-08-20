#ifndef PINS_H
#define PINS_H

#include "driver/gpio.h"

//
// Joystick izquierdo
//
#define JOY0_X         GPIO_NUM_5
#define JOY0_Y         GPIO_NUM_4
#define JOY0_SW        GPIO_NUM_3

//
// Joystick derecho
//
#define JOY1_X         GPIO_NUM_2
#define JOY1_Y         GPIO_NUM_1
#define JOY1_SW        GPIO_NUM_46

//
// Botones centrales
//
#define BTN0           GPIO_NUM_9
#define BTN1           GPIO_NUM_11
#define BTN2           GPIO_NUM_10
#define BTN3           GPIO_NUM_12

//
// Botones laterales
//
#define BTN_L1         GPIO_NUM_42
#define BTN_L2         GPIO_NUM_41
#define BTN_L3         GPIO_NUM_40
#define BTN_L4         GPIO_NUM_39

//
// I2C
//
#define I2C_SDA        GPIO_NUM_6
#define I2C_SCL        GPIO_NUM_7

//
// MPU6050
//
#define MPU_INT        GPIO_NUM_16

//
// OLED SSD1315
//
#define OLED_ADDRESS   0x3C

#endif