#include "buttons.h"
#include "pins.h"

#include "driver/gpio.h"

void buttons_init(void)
{
    gpio_config_t io = {

        .mode = GPIO_MODE_INPUT,

        .pull_up_en = GPIO_PULLUP_ENABLE,

        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE,

        .pin_bit_mask =

        (1ULL<<BTN0)|
        (1ULL<<BTN1)|
        (1ULL<<BTN2)|
        (1ULL<<BTN3)|

        (1ULL<<BTN_L1)|
        (1ULL<<BTN_L2)|
        (1ULL<<BTN_L3)|
        (1ULL<<BTN_L4)|

        (1ULL<<JOY0_SW)|
        (1ULL<<JOY1_SW)

    };

    gpio_config(&io);
}

void buttons_read(button_state_t *b)
{
    b->btn0 = !gpio_get_level(BTN0);
    b->btn1 = !gpio_get_level(BTN1);
    b->btn2 = !gpio_get_level(BTN2);
    b->btn3 = !gpio_get_level(BTN3);

    b->l1 = !gpio_get_level(BTN_L1);
    b->l2 = !gpio_get_level(BTN_L2);
    b->l3 = !gpio_get_level(BTN_L3);
    b->l4 = !gpio_get_level(BTN_L4);

    b->joy0 = !gpio_get_level(JOY0_SW);
    b->joy1 = !gpio_get_level(JOY1_SW);
}