#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>

extern volatile bool btn_a_pressed;
extern volatile bool btn_b_pressed;

void buttons_init(void);

#endif
