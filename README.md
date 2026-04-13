# 💊 MediBox — Dispenser Inteligente de Medicamentos

> Projeto Final — EmbarcaTech Expansão  
> Plataforma: **BitDogLab (RP2040)** | Linguagem: **C/C++** | Protocolo: **MQTT**

---

## 📋 Sobre o Projeto

O **MediBox** é um sistema embarcado que auxilia pacientes e cuidadores no controle rigoroso da administração de medicamentos. O sistema gerencia horários de até **4 medicamentos**, emite alertas sonoros e visuais, registra confirmações via botão físico e publica logs históricos para um servidor remoto via **MQTT/WiFi**.

**Problema abordado:** Cerca de 50% dos pacientes com doenças crônicas não seguem corretamente o regime de medicamentos, gerando complicações evitáveis. O MediBox oferece um lembrete físico eficaz e rastreável.

---

## ✅ Funcionalidades

- ⏰ Alarmes configuráveis para até 4 medicamentos
- 🔔 Alerta sonoro (buzzer PWM) + LED piscando no horário do remédio
- ✅ Confirmação de ingestão via **Botão A** (dentro de 5 minutos)
- ❌ Cancelamento/adiamento via **Botão B**
- 📡 Publicação de log via **MQTT** a cada evento (`tomado` / `adiado` / `não tomado`)
- 📺 Display OLED com hora atual, próximo alarme e histórico
- 🕹️ Navegação por joystick analógico
- 🔄 Reconexão WiFi automática

---

## 🔧 Hardware Necessário

| Componente              | Interface | Pino(s)         |
|-------------------------|-----------|-----------------|
| BitDogLab (RP2040)      | —         | —               |
| Display OLED SSD1306    | I2C       | SDA=14, SCL=15  |
| Buzzer passivo          | PWM       | GPIO 21         |
| Botão A (confirmação)   | GPIO+IRQ  | GPIO 5          |
| Botão B (cancelar)      | GPIO+IRQ  | GPIO 6          |
| Joystick analógico      | ADC       | GPIO 26, 27     |
| LED RGB                 | GPIO      | 13, 11, 12      |
| Módulo WiFi CYW43       | interno   | —               |

---

## 🗂️ Estrutura do Repositório

```
medibox/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── config.h          # Constantes globais (pinos, WiFi, MQTT)
│   ├── alarm.h
│   ├── buttons.h
│   ├── buzzer.h
│   ├── display.h
│   ├── joystick.h
│   ├── mqtt_client.h
│   └── history.h
└── src/
    ├── main.c            # Super-loop principal
    ├── alarm.c           # Gerenciamento de alarmes
    ├── buttons.c         # ISRs + debounce
    ├── buzzer.c          # PWM do buzzer
    ├── display.c         # OLED SSD1306 via I2C
    ├── joystick.c        # Leitura ADC + direção
    ├── mqtt_client.c     # WiFi + MQTT
    └── history.c         # Log em memória flash
```

---

## 🚀 Como compilar e gravar

### Pré-requisitos

- [Pico SDK](https://github.com/raspberrypi/pico-sdk) instalado
- CMake ≥ 3.13
- Arm GCC Toolchain (`arm-none-eabi-gcc`)

### Passo a passo

```bash
# Clone o repositório
git clone https://github.com/SEU_USUARIO/medibox.git
cd medibox

# Configure o caminho do SDK
export PICO_SDK_PATH=/caminho/para/pico-sdk

# Compile
mkdir build && cd build
cmake ..
make -j4
```

### Gravação na placa

1. Segure o botão **BOOTSEL** na BitDogLab e conecte o USB
2. A placa aparece como unidade de armazenamento
3. Copie o arquivo `medibox.uf2` para a unidade
4. A placa reinicia automaticamente e o firmware inicia

---

## ⚙️ Configuração

Edite `include/config.h` antes de compilar:

```c
#define WIFI_SSID   "SUA_REDE"
#define WIFI_PASS   "SUA_SENHA"
#define MQTT_BROKER "broker.hivemq.com"   // ou seu broker
#define MQTT_PORT   1883
```

---

## 📡 Formato da mensagem MQTT

**Tópico:** `medibox/eventos`

```json
{
  "med": "Losartana 50mg",
  "hora": "08:00",
  "status": "tomado",
  "atraso_min": 2
}
```

**Status possíveis:** `tomado` | `adiado` | `nao_tomado`

---

## 📊 Fluxo de operação

```
INÍCIO → Init GPIO → Init Display → WiFi + MQTT → Carregar alarmes
    ↓
SUPER-LOOP:
  Ler RTC → Verificar alarmes
    ├─ Alarme ativo? → Disparar buzzer + LED
    │     ├─ Btn A → confirma → publica MQTT "tomado"
    │     ├─ Btn B → adiar   → publica MQTT "adiado"
    │     └─ Timeout 5min   → publica MQTT "nao_tomado"
    └─ Sem alarme → Joystick → Atualizar display → repetir
```

---

## 📄 Licença

MIT License — livre para uso educacional e não-comercial.

---

## 🙏 Créditos e Ferramentas

- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [lwIP](https://savannah.nongnu.org/projects/lwip/) — stack TCP/IP
- [SSD1306 driver](https://github.com/daschr/pico-ssd1306)
- EmbarcaTech — Programa de Capacitação em Sistemas Embarcados
