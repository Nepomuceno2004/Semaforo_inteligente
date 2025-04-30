#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "generated/ws2812.pio.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#define ledVerde 11
#define ledBlue 12
#define ledVermelho 13

#define botaoA 5

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// pinos da matriz de led
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

volatile bool modoNoturno = false;

bool leds_Aceso[NUM_PIXELS] = {
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    0, 1, 1, 1, 0};

// ativar modo bootcel a partir o botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Converte valores RGB para o formato de 32 bits utilizado pelos LEDs WS2812.
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Atualiza os LEDs da matriz de acordo com o número a ser exibido.
void set_one_led(uint8_t r, uint8_t g, uint8_t b, bool led_buffer[])
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0); // Desliga os LEDs com zero no buffer
        }
    }
}

void vBotaoA()
{
    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);

    while (true)
    {
        if (!gpio_get(botaoA))
        {
            modoNoturno = !modoNoturno;

            // Aguarda o botão ser solto
            while (!gpio_get(botaoA))
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            // Pequeno atraso para debounce
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

void vMatrizTask()
{
    // inicialização da matriz de led
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    while (true)
    {
        if (!modoNoturno)
        {
            set_one_led(0, 2, 0, leds_Aceso); // manda a cor verde para a matriz de led
            vTaskDelay(pdMS_TO_TICKS(3000));
            set_one_led(2, 2, 0, leds_Aceso); // manda a cor amarela para a matriz de led
            vTaskDelay(pdMS_TO_TICKS(2000));
            set_one_led(2, 0, 0, leds_Aceso); // manda a cor vermelha para a matriz de led
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
        else
        {
            set_one_led(2, 2, 0, leds_Aceso); // manda a cor vermelha para a matriz de led
            vTaskDelay(pdMS_TO_TICKS(1000));
            for (int i = 0; i < NUM_PIXELS; i++)
            {
                put_pixel(0); // Desliga todos LEDs
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}


void vSemaforoTask()
{
    gpio_init(ledVerde);
    gpio_set_dir(ledVerde, GPIO_OUT);

    gpio_init(ledBlue);
    gpio_set_dir(ledBlue, GPIO_OUT);

    gpio_init(ledVermelho);
    gpio_set_dir(ledVermelho, GPIO_OUT);

    while (true)
    {
        if (!modoNoturno)
        {
            gpio_put(ledVerde, true);
            vTaskDelay(pdMS_TO_TICKS(3000));
            gpio_put(ledVermelho, true);
            vTaskDelay(pdMS_TO_TICKS(2000));
            gpio_put(ledVerde, false);
            vTaskDelay(pdMS_TO_TICKS(3000));
            gpio_put(ledVermelho, false);
        }
        else
        {
            gpio_put(ledVerde, true);
            gpio_put(ledVermelho, true);
            vTaskDelay(pdMS_TO_TICKS(1000));
            gpio_put(ledVerde, false);
            gpio_put(ledVermelho, false);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

int main()
{
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    stdio_init_all();

    xTaskCreate(vBotaoA, "Display Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vSemaforoTask, "Semaforo Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vMatrizTask, "Matriz Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}
