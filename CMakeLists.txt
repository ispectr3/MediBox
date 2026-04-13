cmake_minimum_required(VERSION 3.13)

# Inicializa o SDK do Pico W
set(PICO_BOARD pico_w)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(medibox C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(medibox
    src/main.c
    src/alarm.c
    src/buttons.c
    src/buzzer.c
    src/display.c
    src/joystick.c
    src/mqtt_client.c
    src/history.c
)

target_include_directories(medibox PRIVATE include)

target_link_libraries(medibox
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    hardware_adc
    hardware_i2c
    hardware_pwm
    hardware_rtc
    hardware_flash
)

# Habilita saída USB serial
pico_enable_stdio_usb(medibox 1)
pico_enable_stdio_uart(medibox 0)

# Gera .uf2 para upload via drag-and-drop
pico_add_extra_outputs(medibox)
