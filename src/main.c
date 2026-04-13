/**
 * MediBox — Dispenser Inteligente de Medicamentos
 * Plataforma: BitDogLab (RP2040)
 * Projeto Final EmbarcaTech Expansão
 */

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/rtc.h"
#include "config.h"
#include "alarm.h"
#include "buttons.h"
#include "buzzer.h"
#include "display.h"
#include "joystick.h"
#include "mqtt_client.h"

// ─── Flags externas (definidas em buttons.c) ──────────────────────────────────
extern volatile bool btn_a_pressed;
extern volatile bool btn_b_pressed;

// ─── Protótipos ───────────────────────────────────────────────────────────────
static void system_init(void);
static void get_rtc_time(uint8_t *hora, uint8_t *minuto);

// ─── Main ─────────────────────────────────────────────────────────────────────
int main(void) {
    system_init();

    uint8_t hora = 0, minuto = 0;

    while (true) {
        // 1. Ler hora atual do RTC
        get_rtc_time(&hora, &minuto);

        // 2. Verificar alarmes
        alarm_check(hora, minuto);

        // 3. Processar botões (flags definidas pela ISR)
        if (btn_a_pressed) {
            btn_a_pressed = false;
            alarm_confirm();   // Tomou o medicamento
        }
        if (btn_b_pressed) {
            btn_b_pressed = false;
            alarm_dismiss();   // Adiou / cancelou
        }

        // 4. Joystick → navegação de menu (só fora de alarme ativo)
        if (!alarm_is_ringing()) {
            joystick_dir_t dir = joystick_read();
            display_navigate(dir);
        }

        // 5. Atualizar display
        display_update(hora, minuto);

        // 6. Manter conexão MQTT viva
        mqtt_poll();

        sleep_ms(100);   // ~10 Hz
    }

    return 0;
}

// ─── Inicialização ────────────────────────────────────────────────────────────
static void system_init(void) {
    stdio_init_all();

    // ADC para joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    // Módulos
    buttons_init();
    buzzer_init();
    display_init();
    joystick_init();
    alarm_init();

    // Conecta WiFi e broker MQTT
    mqtt_init();

    // RTC com hora de compilação (ajustar via NTP em versão futura)
    rtc_init();
    datetime_t t = { .year=2025, .month=1, .day=1,
                     .dotw=3, .hour=7, .min=59, .sec=50 };
    rtc_set_datetime(&t);

    display_splash();   // Tela de boas-vindas
}

static void get_rtc_time(uint8_t *hora, uint8_t *minuto) {
    datetime_t t;
    rtc_get_datetime(&t);
    *hora   = (uint8_t)t.hour;
    *minuto = (uint8_t)t.min;
}
