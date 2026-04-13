#include "mqtt_client.h"
#include "config.h"
#include "alarm.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include <stdio.h>
#include <string.h>

// ─── Estado ───────────────────────────────────────────────────────────────────

static mqtt_client_t *client = NULL;
static bool connected = false;

// ─── Callbacks ───────────────────────────────────────────────────────────────

static void mqtt_connection_cb(mqtt_client_t *c, void *arg, mqtt_connection_status_t status) {
    (void)c; (void)arg;
    connected = (status == MQTT_CONNECT_ACCEPTED);
    printf("[MQTT] %s\n", connected ? "Conectado" : "Desconectado");
}

static void mqtt_pub_request_cb(void *arg, err_t err) {
    (void)arg;
    if (err != ERR_OK)
        printf("[MQTT] Falha ao publicar: %d\n", err);
}

// ─── Conexão WiFi ─────────────────────────────────────────────────────────────

static bool wifi_connect(void) {
    if (cyw43_arch_init()) {
        printf("[WiFi] Falha ao inicializar\n");
        return false;
    }
    cyw43_arch_enable_sta_mode();

    printf("[WiFi] Conectando a %s...\n", WIFI_SSID);
    int ret = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000);

    if (ret != 0) {
        printf("[WiFi] Falha: %d\n", ret);
        return false;
    }
    printf("[WiFi] Conectado!\n");
    return true;
}

// ─── Conexão MQTT ─────────────────────────────────────────────────────────────

static void mqtt_connect(void) {
    client = mqtt_client_new();
    if (!client) return;

    struct mqtt_connect_client_info_t ci = {
        .client_id   = MQTT_CLIENT_ID,
        .client_user = NULL,
        .client_pass = NULL,
        .keep_alive  = 60,
        .will_topic  = NULL,
    };

    ip_addr_t broker_ip;
    // Resolve hostname → IP (simplificado; para produção usar dns_gethostbyname)
    ipaddr_aton(MQTT_BROKER, &broker_ip);

    mqtt_client_connect(client, &broker_ip, MQTT_PORT,
                        mqtt_connection_cb, NULL, &ci);
}

// ─── API pública ──────────────────────────────────────────────────────────────

void mqtt_init(void) {
    if (!wifi_connect()) return;
    mqtt_connect();
}

void mqtt_poll(void) {
    cyw43_arch_poll();

    // Reconexão automática
    if (!connected && client) {
        static uint32_t last_retry = 0;
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_retry > 10000) {  // tenta a cada 10s
            last_retry = now;
            mqtt_connect();
        }
    }
}

void mqtt_publish_event(alarm_t *alarm) {
    if (!connected || !client) {
        printf("[MQTT] Sem conexão, evento descartado\n");
        return;
    }

    const char *status_str;
    switch (alarm->ultimo_status) {
        case STATUS_TOMADO:     status_str = "tomado";     break;
        case STATUS_ADIADO:     status_str = "adiado";     break;
        case STATUS_NAO_TOMADO: status_str = "nao_tomado"; break;
        default:                status_str = "pendente";   break;
    }

    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"med\":\"%s\",\"hora\":\"%02d:%02d\",\"status\":\"%s\"}",
             alarm->nome, alarm->hora, alarm->minuto, status_str);

    mqtt_publish(client, MQTT_TOPIC,
                 payload, strlen(payload),
                 1,    // QoS 1
                 0,    // retain = false
                 mqtt_pub_request_cb, NULL);

    printf("[MQTT] Publicado: %s\n", payload);
}
