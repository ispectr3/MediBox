#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "alarm.h"

void mqtt_init(void);
void mqtt_poll(void);
void mqtt_publish_event(alarm_t *alarm);

#endif
