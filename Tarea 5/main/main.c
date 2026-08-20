#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "pins.h"

#include "buttons.h"
#include "joystick.h"
#include "mpu6050.h"

#include "ssd1306.h"

SSD1306_t display;

void app_main(void)
{
    buttons_init();

    joystick_init();

    mpu6050_init();

    i2c_master_init(
        &display,
        I2C_SDA,
        I2C_SCL,
        -1
    );

    ssd1306_init(
        &display,
        128,
        64
    );

    ssd1306_clear_screen(
        &display,
        false
    );

    ssd1306_display_text(
        &display,
        0,
        "GAMEPAD",
        7,
        false
    );

    while(1)
    {

    }
}