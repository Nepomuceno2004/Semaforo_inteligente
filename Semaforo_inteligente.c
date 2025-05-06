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
#include "lib/matrizLed.h"
#include "buzzer.h"

#define ledVerde 11
#define ledVermelho 13

#define botaoA 5

// pinos da matriz de led
#define IS_RGBW false
#define WS2812_PIN 7

#define BUZZER_PIN 21

volatile bool modoNoturno = true;
ssd1306_t ssd; // Inicializa a estrutura do display

bool leds_Aceso[NUM_PIXELS] = {
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    0, 1, 1, 1, 0};

void vBotaoATask()
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
            vTaskDelay(pdMS_TO_TICKS(150));
        }
    }
}

void vDisplayTask()
{
    initDisplay(&ssd);

    while (true)
    {
        if (!modoNoturno)
        {
            ssd1306_fill(&ssd, false);
            desenhar(&ssd, carro);

            vTaskDelay(pdMS_TO_TICKS(2950));

            ssd1306_fill(&ssd, false);
            desenhar(&ssd, atencao);

            vTaskDelay(pdMS_TO_TICKS(1950));

            ssd1306_fill(&ssd, false);
            desenhar(&ssd, humano);

            vTaskDelay(pdMS_TO_TICKS(3900));
        }
        else
        {
            desenhar(&ssd, atencao);
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
            vTaskDelay(pdMS_TO_TICKS(4000));
        }
        else
        {
            set_one_led(2, 2, 0, leds_Aceso); // manda a cor amarela para a matriz de led
            vTaskDelay(pdMS_TO_TICKS(1000));
            for (int i = 0; i < NUM_PIXELS; i++)
            {
                put_pixel(0); // Desliga todos LEDs
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void vBuzzerTask()
{

    buzzer_init(BUZZER_PIN);
    while (true)
    {
        if (!modoNoturno)
        {
            for (int i = 0; i < 3; i++)
            {
                modo_verde(BUZZER_PIN);
            }

            for (int i = 0; i < 2; i++)
            {
                modo_amarelo(BUZZER_PIN);
            }

            for (int i = 0; i < 2; i++)
            {
                modo_vermelho(BUZZER_PIN);
            }
        }
        else
        {
            buzzer_noturno(BUZZER_PIN);
        }
    }
}

void vLedRGBTask()
{

    gpio_init(ledVerde);
    gpio_set_dir(ledVerde, GPIO_OUT);

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

            vTaskDelay(pdMS_TO_TICKS(4000));
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
    stdio_init_all();

    xTaskCreate(vBotaoATask, "Botao Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vLedRGBTask, "Led RGB Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vMatrizTask, "Matriz Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vDisplayTask, "Display Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vBuzzerTask, "Buzzer Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}
