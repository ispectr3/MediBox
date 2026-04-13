#include "history.h"
#include "config.h"
#include <string.h>
#include <stdio.h>

// ─── Buffer circular em RAM ───────────────────────────────────────────────────
// (Para persistência real, gravar em flash com hardware_flash após cada evento)

static history_entry_t entries[HISTORY_MAX];
static uint8_t count = 0;
static uint8_t head  = 0;  // índice mais antigo (buffer circular)

void history_init(void) {
    memset(entries, 0, sizeof(entries));
    count = 0;
    head  = 0;
    printf("[HISTORY] Inicializado\n");
}

void history_add(alarm_t *alarm) {
    uint8_t idx = (head + count) % HISTORY_MAX;

    strncpy(entries[idx].nome, alarm->nome, sizeof(entries[idx].nome) - 1);
    entries[idx].hora    = alarm->hora;
    entries[idx].minuto  = alarm->minuto;
    entries[idx].status  = alarm->ultimo_status;

    if (count < HISTORY_MAX) {
        count++;
    } else {
        // Buffer cheio — sobrescreve o mais antigo
        head = (head + 1) % HISTORY_MAX;
    }

    printf("[HISTORY] Adicionado: %s %02d:%02d status=%d\n",
           alarm->nome, alarm->hora, alarm->minuto, alarm->ultimo_status);
}

history_entry_t* history_get(uint8_t idx) {
    if (idx >= count) return NULL;
    uint8_t real_idx = (head + idx) % HISTORY_MAX;
    return &entries[real_idx];
}

uint8_t history_count(void) {
    return count;
}
