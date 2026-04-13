#ifndef DISPLAY_H
#define DISPLAY_H

#include "alarm.h"
#include "joystick.h"

void display_init(void);
void display_splash(void);
void display_update(uint8_t hora, uint8_t minuto);
void display_show_alarm(alarm_t *alarm);
void display_navigate(joystick_dir_t dir);

#endif
