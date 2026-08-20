#ifndef OLED_H
#define OLED_H

#include <stdint.h>

#define OLED_WIDTH     128
#define OLED_HEIGHT    64

void oled_init(void);

void oled_clear(void);

void oled_update(void);

void oled_draw_pixel(uint8_t x, uint8_t y);

void oled_draw_char(uint8_t x,
                    uint8_t y,
                    char c);

void oled_draw_string(uint8_t x,
                      uint8_t y,
                      const char *text);

#endif