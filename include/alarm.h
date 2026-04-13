#ifndef ALARM_H
#define ALARM_H

#include <stdint.h>
#include <stdbool.h>

// ─── Tipos ────────────────────────────────────────────────────────────────────

typedef enum {
    STATUS_PENDENTE,
    STATUS_TOMADO,
    STATUS_ADIADO,
    STATUS_NAO_TOMADO
} alarm_status_t;

typedef struct {
    char     nome[32];       // Ex: "Losartana 50mg"
    uint8_t  hora;           // 0-23
    uint8_t  minuto;         // 0-59
    bool     ativo;
    alarm_status_t ultimo_status;
    uint32_t ultimo_disparo; // timestamp unix
} alarm_t;

// ─── API ──────────────────────────────────────────────────────────────────────

void     alarm_init(void);
void     alarm_check(uint8_t hora_atual, uint8_t min_atual);
bool     alarm_is_ringing(void);
alarm_t* alarm_get_active(void);
void     alarm_confirm(void);    // Botão A → STATUS_TOMADO
void     alarm_dismiss(void);    // Botão B → STATUS_ADIADO
void     alarm_set(uint8_t idx, const char* nome, uint8_t hora, uint8_t min);

#endif // ALARM_H
