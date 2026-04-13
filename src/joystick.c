#include "joystick.h"
#include "config.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"

void joystick_init(void) {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);  // ADC0
    adc_gpio_init(JOYSTICK_Y_PIN);  // ADC1
}

joystick_dir_t joystick_read(void) {
    adc_select_input(0);
    uint16_t x = adc_read();
    adc_select_input(1);
    uint16_t y = adc_read();

    // Deadzone central
    if (x > JOY_PRESS - JOY_THRESHOLD &&
        x < JOY_PRESS + JOY_THRESHOLD &&
        y > JOY_PRESS - JOY_THRESHOLD &&
        y < JOY_PRESS + JOY_THRESHOLD) {
        return JOY_NONE;
    }

    // Eixo predominante
    int dx = (int)x - JOY_PRESS;
    int dy = (int)y - JOY_PRESS;

    if (dy < -JOY_THRESHOLD) return JOY_UP;
    if (dy >  JOY_THRESHOLD) return JOY_DOWN;
    if (dx < -JOY_THRESHOLD) return JOY_LEFT;
    if (dx >  JOY_THRESHOLD) return JOY_RIGHT;

    return JOY_NONE;
}
