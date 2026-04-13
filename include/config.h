#ifndef CONFIG_H
#define CONFIG_H

// ─── Pinos GPIO ───────────────────────────────────────────────────────────────
#define BTN_A_PIN       5
#define BTN_B_PIN       6
#define BUZZER_PIN      21
#define LED_R_PIN       13
#define LED_G_PIN       11
#define LED_B_PIN       12
#define JOYSTICK_X_PIN  26   // ADC0
#define JOYSTICK_Y_PIN  27   // ADC1
#define I2C_SDA_PIN     14
#define I2C_SCL_PIN     15
#define OLED_ADDR       0x3C

// ─── WiFi / MQTT ─────────────────────────────────────────────────────────────
#define WIFI_SSID       "SUA_REDE"
#define WIFI_PASS       "SUA_SENHA"
#define MQTT_BROKER     "broker.hivemq.com"
#define MQTT_PORT       1883
#define MQTT_TOPIC      "medibox/eventos"
#define MQTT_CLIENT_ID  "medibox_rp2040"

// ─── Alarmes ─────────────────────────────────────────────────────────────────
#define MAX_ALARMS      4
#define CONFIRM_WINDOW  300   // segundos para confirmar (5 min)

// ─── PWM Buzzer ──────────────────────────────────────────────────────────────
#define BUZZER_FREQ_ALERT   1000  // Hz
#define BUZZER_FREQ_CONFIRM  800  // Hz
#define BUZZER_DUTY         0.5f

// ─── Joystick ────────────────────────────────────────────────────────────────
#define JOY_CENTER      2048
#define JOY_THRESHOLD   800

#endif // CONFIG_H
