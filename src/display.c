#include "display.h"
#include "config.h"
#include "alarm.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <string.h>

// ─── Driver mínimo SSD1306 128x64 via I2C ────────────────────────────────────

#define OLED_W 128
#define OLED_H  64
#define OLED_PAGES (OLED_H / 8)

static uint8_t framebuf[OLED_PAGES][OLED_W];

// Fonte 5x8 — ASCII 32..126 (subconjunto básico)
static const uint8_t font5x8[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // ' '
    {0x00,0x00,0x5F,0x00,0x00}, // '!'
    {0x3E,0x51,0x49,0x45,0x3E}, // '0'
    {0x00,0x42,0x7F,0x40,0x00}, // '1'
    {0x42,0x61,0x51,0x49,0x46}, // '2'
    {0x21,0x41,0x45,0x4B,0x31}, // '3'
    {0x18,0x14,0x12,0x7F,0x10}, // '4'
    {0x27,0x45,0x45,0x45,0x39}, // '5'
    {0x3C,0x4A,0x49,0x49,0x30}, // '6'
    {0x01,0x71,0x09,0x05,0x03}, // '7'
    {0x36,0x49,0x49,0x49,0x36}, // '8'
    {0x06,0x49,0x49,0x29,0x1E}, // '9'
    {0x00,0x36,0x36,0x00,0x00}, // ':'
    // Letras maiúsculas A-Z (simplificado — adicionar conforme necessário)
    {0x7E,0x11,0x11,0x11,0x7E}, // 'A'
    {0x7F,0x49,0x49,0x49,0x36}, // 'B'
    {0x3E,0x41,0x41,0x41,0x22}, // 'C'
    {0x7F,0x41,0x41,0x22,0x1C}, // 'D'
    {0x7F,0x49,0x49,0x49,0x41}, // 'E'
    {0x7F,0x09,0x09,0x09,0x01}, // 'F'
    {0x3E,0x41,0x49,0x49,0x7A}, // 'G'
    {0x7F,0x08,0x08,0x08,0x7F}, // 'H'
    {0x00,0x41,0x7F,0x41,0x00}, // 'I'
    {0x20,0x40,0x41,0x3F,0x01}, // 'J'
    {0x7F,0x08,0x14,0x22,0x41}, // 'K'
    {0x7F,0x40,0x40,0x40,0x40}, // 'L'
    {0x7F,0x02,0x0C,0x02,0x7F}, // 'M'
    {0x7F,0x04,0x08,0x10,0x7F}, // 'N'
    {0x3E,0x41,0x41,0x41,0x3E}, // 'O'
    {0x7F,0x09,0x09,0x09,0x06}, // 'P'
    {0x3E,0x41,0x51,0x21,0x5E}, // 'Q'
    {0x7F,0x09,0x19,0x29,0x46}, // 'R'
    {0x46,0x49,0x49,0x49,0x31}, // 'S'
    {0x01,0x01,0x7F,0x01,0x01}, // 'T'
    {0x3F,0x40,0x40,0x40,0x3F}, // 'U'
    {0x1F,0x20,0x40,0x20,0x1F}, // 'V'
    {0x3F,0x40,0x38,0x40,0x3F}, // 'W'
    {0x63,0x14,0x08,0x14,0x63}, // 'X'
    {0x07,0x08,0x70,0x08,0x07}, // 'Y'
    {0x61,0x51,0x49,0x45,0x43}, // 'Z'
};

// Envia comando ao SSD1306
static void oled_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    i2c_write_blocking(i2c1, OLED_ADDR, buf, 2, false);
}

// Envia framebuffer completo
static void oled_flush(void) {
    oled_cmd(0x21); oled_cmd(0); oled_cmd(127);
    oled_cmd(0x22); oled_cmd(0); oled_cmd(7);

    uint8_t buf[OLED_W + 1];
    buf[0] = 0x40;
    for (int p = 0; p < OLED_PAGES; p++) {
        memcpy(buf + 1, framebuf[p], OLED_W);
        i2c_write_blocking(i2c1, OLED_ADDR, buf, OLED_W + 1, false);
    }
}

static void oled_clear(void) {
    memset(framebuf, 0, sizeof(framebuf));
}

// Desenha caractere na posição (col, page)
static void draw_char(uint8_t col, uint8_t page, char c) {
    // Mapeia caractere para índice da fonte
    int idx = -1;
    if (c == ' ') idx = 0;
    else if (c >= '0' && c <= '9') idx = 2 + (c - '0');
    else if (c == ':') idx = 12;
    else if (c >= 'A' && c <= 'Z') idx = 13 + (c - 'A');

    if (idx < 0 || col + 5 > OLED_W) return;
    for (int i = 0; i < 5; i++) {
        if (page < OLED_PAGES)
            framebuf[page][col + i] = font5x8[idx][i];
    }
    if (col + 5 < OLED_W) framebuf[page][col + 5] = 0x00; // espaço
}

static void draw_str(uint8_t col, uint8_t page, const char *s) {
    while (*s) {
        draw_char(col, page, *s++);
        col += 6;
        if (col >= OLED_W) break;
    }
}

// ─── Telas ────────────────────────────────────────────────────────────────────

typedef enum { SCREEN_IDLE, SCREEN_ALARM, SCREEN_HISTORY } screen_t;
static screen_t current_screen = SCREEN_IDLE;
static int      history_scroll = 0;

void display_init(void) {
    i2c_init(i2c1, 400000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    sleep_ms(100);

    // Sequência de inicialização SSD1306
    const uint8_t init_cmds[] = {
        0xAE, 0x20, 0x00, 0x40, 0xA1, 0xC8,
        0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1,
        0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };
    for (size_t i = 0; i < sizeof(init_cmds); i++)
        oled_cmd(init_cmds[i]);

    oled_clear();
    oled_flush();
}

void display_splash(void) {
    oled_clear();
    draw_str(28, 2, "MEDIBOX");
    draw_str(16, 4, "INICIANDO");
    oled_flush();
    sleep_ms(2000);
}

void display_update(uint8_t hora, uint8_t minuto) {
    if (current_screen != SCREEN_IDLE) return;

    oled_clear();
    char buf[20];

    // Hora grande
    snprintf(buf, sizeof(buf), "%02d:%02d", hora, minuto);
    draw_str(36, 1, buf);

    draw_str(0, 3, "PROXIMO");
    draw_str(0, 4, "MEDICAMENTO");

    oled_flush();
}

void display_show_alarm(alarm_t *alarm) {
    current_screen = SCREEN_ALARM;
    oled_clear();
    draw_str(4, 0, "HORA DO REMEDIO");
    // Nome do medicamento (primeiros 20 chars)
    draw_str(0, 2, alarm->nome);
    draw_str(0, 4, "A  TOMEI");
    draw_str(0, 5, "B  ADIAR");
    oled_flush();
}

void display_navigate(joystick_dir_t dir) {
    if (dir == JOY_UP || dir == JOY_DOWN) {
        current_screen = SCREEN_HISTORY;
        if (dir == JOY_DOWN) history_scroll++;
        if (dir == JOY_UP && history_scroll > 0) history_scroll--;

        oled_clear();
        draw_str(0, 0, "HISTORICO");
        // Histórico real viria do módulo history.c
        draw_str(0, 2, "08:00 TOMADO");
        draw_str(0, 3, "12:00 ADIADO");
        draw_str(0, 4, "19:30 NAO TOM");
        oled_flush();
    } else if (dir == JOY_PRESS) {
        current_screen = SCREEN_IDLE;
    }
}
