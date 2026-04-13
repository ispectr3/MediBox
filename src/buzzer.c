#include "buzzer.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

static uint slice_num;

void buzzer_init(void) {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_enabled(slice_num, false);
}

// Define frequência e duty cycle no PWM
static void buzzer_set_freq(uint freq_hz) {
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / freq_hz / 4096 + 1;
    if (divider16 / 16 == 0) divider16 = 16;
    uint32_t wrap = clock * 16 / divider16 / freq_hz - 1;
    pwm_set_clkdiv_int_frac(slice_num, divider16 / 16, divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_gpio_level(BUZZER_PIN, wrap / 2);  // 50% duty
}

void buzzer_alert(void) {
    buzzer_set_freq(BUZZER_FREQ_ALERT);
    pwm_set_enabled(slice_num, true);
}

void buzzer_confirm(void) {
    // Dois bipes curtos de confirmação
    buzzer_set_freq(BUZZER_FREQ_CONFIRM);
    pwm_set_enabled(slice_num, true);
    sleep_ms(120);
    pwm_set_enabled(slice_num, false);
    sleep_ms(80);
    pwm_set_enabled(slice_num, true);
    sleep_ms(120);
    pwm_set_enabled(slice_num, false);
}

void buzzer_stop(void) {
    pwm_set_enabled(slice_num, false);
}
