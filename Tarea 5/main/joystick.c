#include "joystick.h"
#include "pins.h"

#include "esp_adc/adc_oneshot.h"

static adc_oneshot_unit_handle_t adc1;

void joystick_init(void)
{
    adc_oneshot_unit_init_cfg_t init = {

        .unit_id = ADC_UNIT_1

    };

    adc_oneshot_new_unit(&init, &adc1);

    adc_oneshot_chan_cfg_t cfg = {

        .atten = ADC_ATTEN_DB_12,

        .bitwidth = ADC_BITWIDTH_DEFAULT

    };

    adc_oneshot_config_channel(adc1, ADC_CHANNEL_4, &cfg);
    adc_oneshot_config_channel(adc1, ADC_CHANNEL_3, &cfg);
    adc_oneshot_config_channel(adc1, ADC_CHANNEL_2, &cfg);
    adc_oneshot_config_channel(adc1, ADC_CHANNEL_1, &cfg);
}

void joystick_read(joystick_t *joy)
{
    adc_oneshot_read(adc1, ADC_CHANNEL_4, &joy->lx);
    adc_oneshot_read(adc1, ADC_CHANNEL_3, &joy->ly);
    adc_oneshot_read(adc1, ADC_CHANNEL_2, &joy->rx);
    adc_oneshot_read(adc1, ADC_CHANNEL_1, &joy->ry);
}