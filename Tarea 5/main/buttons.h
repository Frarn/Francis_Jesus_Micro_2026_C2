#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>

typedef struct
{
    bool btn0;
    bool btn1;
    bool btn2;
    bool btn3;

    bool l1;
    bool l2;
    bool l3;
    bool l4;

    bool joy0;
    bool joy1;

} button_state_t;

void buttons_init(void);

void buttons_read(button_state_t *buttons);

#endif