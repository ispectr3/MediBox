#ifndef JOYSTICK_H
#define JOYSTICK_H

typedef enum {
    JOY_NONE,
    JOY_UP,
    JOY_DOWN,
    JOY_LEFT,
    JOY_RIGHT,
    JOY_PRESS
} joystick_dir_t;

void           joystick_init(void);
joystick_dir_t joystick_read(void);

#endif
