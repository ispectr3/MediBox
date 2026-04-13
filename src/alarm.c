#include "alarm.h"
#include "config.h"
#include "buzzer.h"
#include "display.h"
#include "mqtt_client.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>

// ─── Estado interno ───────────────────────────────────────────────────────────

static alarm_t   alarms[MAX_ALARMS];
static int       active_idx = -1;
static uint32_t  alarm_start_ts = 0;

// ─── Implementação ────────────────────────────────────────────────────────────

void alarm_init(void) {
    memset(alarms, 0, sizeof(alarms));

    // Pré-configura medicamentos de exemplo
    alarm_set(0, "Losartana 50mg",   8,  0);
    alarm_set(1, "Metformina 500mg", 12, 0);
    alarm_set(2, "AAS 100mg",        19, 30);
}

void alarm_set(uint8_t idx, const char* nome, uint8_t hora, uint8_t min) {
    if (idx >= MAX_ALARMS) return;
    strncpy(alarms[idx].nome, nome, sizeof(alarms[idx].nome) - 1);
    alarms[idx].hora   = hora;
    alarms[idx].minuto = min;
    alarms[idx].ativo  = true;
    alarms[idx].ultimo_status = STATUS_PENDENTE;
}

void alarm_check(uint8_t hora_atual, uint8_t min_atual) {
    // Se já há alarme tocando, verifica timeout (5 min)
    if (active_idx >= 0) {
        uint32_t agora = to_ms_since_boot(get_absolute_time()) / 1000;
        if ((agora - alarm_start_ts) > CONFIRM_WINDOW) {
            alarms[active_idx].ultimo_status = STATUS_NAO_TOMADO;
            mqtt_publish_event(&alarms[active_idx]);
            buzzer_stop();
            active_idx = -1;
        }
        return;
    }

    // Verifica todos os alarmes
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (!alarms[i].ativo) continue;
        if (alarms[i].hora == hora_atual && alarms[i].minuto == min_atual) {
            active_idx    = i;
            alarm_start_ts = to_ms_since_boot(get_absolute_time()) / 1000;
            alarms[i].ultimo_status = STATUS_PENDENTE;

            buzzer_alert();
            display_show_alarm(&alarms[i]);
        }
    }
}

bool alarm_is_ringing(void) {
    return active_idx >= 0;
}

alarm_t* alarm_get_active(void) {
    if (active_idx < 0) return NULL;
    return &alarms[active_idx];
}

void alarm_confirm(void) {
    if (active_idx < 0) return;
    alarms[active_idx].ultimo_status = STATUS_TOMADO;
    mqtt_publish_event(&alarms[active_idx]);
    buzzer_confirm();
    active_idx = -1;
}

void alarm_dismiss(void) {
    if (active_idx < 0) return;
    alarms[active_idx].ultimo_status = STATUS_ADIADO;
    mqtt_publish_event(&alarms[active_idx]);
    buzzer_stop();
    active_idx = -1;
}
