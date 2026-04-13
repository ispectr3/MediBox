#include "buttons.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// ─── Flags voláteis (consumidas no super-loop) ────────────────────────────────
volatile bool btn_a_pressed = false;
volatile bool btn_b_pressed = false;

// ─── Debounce ─────────────────────────────────────────────────────────────────
#define DEBOUNCE_MS 50

static uint32_t last_a_ms = 0;
static uint32_t last_b_ms = 0;

// ─── ISR compartilhada ────────────────────────────────────────────────────────
static void gpio_isr(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (gpio == BTN_A_PIN && (now - last_a_ms) > DEBOUNCE_MS) {
        btn_a_pressed = true;
        last_a_ms = now;
    }
    if (gpio == BTN_B_PIN && (now - last_b_ms) > DEBOUNCE_MS) {
        btn_b_pressed = true;
        last_b_ms = now;
    }
}

// ─── Inicialização ────────────────────────────────────────────────────────────
void buttons_init(void) {
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);

    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_isr);
    gpio_set_irq_enabled(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true);
}
