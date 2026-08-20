#ifndef JOYSTICK_H
#define JOYSTICK_H

typedef struct
{
    int lx;
    int ly;

    int rx;
    int ry;

} joystick_t;

void joystick_init(void);

void joystick_read(joystick_t *joy);

#endif