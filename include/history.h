#ifndef HISTORY_H
#define HISTORY_H

#include "alarm.h"
#include <stdint.h>

#define HISTORY_MAX 20

typedef struct {
    char   nome[32];
    uint8_t hora;
    uint8_t minuto;
    alarm_status_t status;
} history_entry_t;

void             history_init(void);
void             history_add(alarm_t *alarm);
history_entry_t* history_get(uint8_t idx);
uint8_t          history_count(void);

#endif
