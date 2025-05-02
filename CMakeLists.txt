# Generated Cmake Pico project file
# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
set(FREERTOS_KERNEL_PATH "C:/FreeRTOS-Kernel")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(Semaforo_inteligente C CXX ASM)

pico_sdk_init()

include_directories(${CMAKE_SOURCE_DIR}/lib)

        add_executable(Semaforo_inteligente 
        Semaforo_inteligente.c 
        lib/ssd1306.c # Biblioteca para o display OLED
        lib/matrizLed.c
        lib/buzzer.c
        )

pico_set_program_name(Semaforo_inteligente "Semaforo_inteligente")
pico_set_program_version(Semaforo_inteligente "0.1")

pico_enable_stdio_uart(Semaforo_inteligente 0)
pico_enable_stdio_usb(Semaforo_inteligente 0)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(Semaforo_inteligente
        ${CMAKE_CURRENT_LIST_DIR}/lib/ws2812.pio OUTPUT_DIR 
        ${CMAKE_CURRENT_LIST_DIR}/generated)

target_link_libraries(Semaforo_inteligente
        pico_stdlib
        FreeRTOS-Kernel 
        FreeRTOS-Kernel-Heap4
        hardware_pio
        hardware_pwm
        hardware_i2c
        )

target_include_directories(Semaforo_inteligente PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(Semaforo_inteligente)