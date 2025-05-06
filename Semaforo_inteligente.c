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

// pinos do led rgb (o led azul não foi necessário)
#define ledVerde 11
#define ledVermelho 13

// pino do botão A
#define botaoA 5

// pinos da matriz de led
#define IS_RGBW false
#define WS2812_PIN 7

// pino do buzzer
#define BUZZER_PIN 21

// flag para alterar os modos (normal/noturno)
volatile bool modoNoturno = false;

// Inicializa a estrutura do display
ssd1306_t ssd;

// matriz que define quais leds acendem
bool leds_Aceso[NUM_PIXELS] = {
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    0, 1, 1, 1, 0};

// task que controla o botão
void vBotaoATask()
{
    // inicializa e configura o botão
    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);

    while (true)
    {
        if (!gpio_get(botaoA)) // quando há o pressionamento
        {
            modoNoturno = !modoNoturno; // altera o modo

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

// task que controla o display
void vDisplayTask()
{
    initDisplay(&ssd); // inicializa o display

    while (true)
    {
        ssd1306_fill(&ssd, false); // limpa os pixels
        if (!modoNoturno)
        {
            desenhar(&ssd, carro);           // envia o desenho do carro
            vTaskDelay(pdMS_TO_TICKS(2900)); // espera a troca de sinal
            ssd1306_fill(&ssd, false);

            desenhar(&ssd, atencao); // envia o desenho de atenção
            vTaskDelay(pdMS_TO_TICKS(1900));
            ssd1306_fill(&ssd, false);

            desenhar(&ssd, humano); // envia o desenho do humano
            vTaskDelay(pdMS_TO_TICKS(3900));
        }
        else
        {
            desenhar(&ssd, atencao); // envia o desenho de atenção
            vTaskDelay(pdMS_TO_TICKS(950));
        }
    }
}
//função para desligar todos os leds da matriz
void desligarMatriz()
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        put_pixel(0); // Desliga um LED da matriz por vez
    }
    return;
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
            vTaskDelay(pdMS_TO_TICKS(3000));  // espera a troca de sinal

            set_one_led(2, 2, 0, leds_Aceso); // manda a cor amarela para a matriz de led
            vTaskDelay(pdMS_TO_TICKS(2000));

            set_one_led(2, 0, 0, leds_Aceso); // manda a cor vermelha para a matriz de led
            vTaskDelay(pdMS_TO_TICKS(4000));
        }
        else
        {
            set_one_led(2, 2, 0, leds_Aceso); // manda a cor amarela para a matriz de led
            vTaskDelay(pdMS_TO_TICKS(950));
            desligarMatriz(); // desliga todos os leds
            vTaskDelay(pdMS_TO_TICKS(950));
        }
    }
}

void vBuzzerTask()
{

    buzzer_init(BUZZER_PIN); // inicializa o buzzer

    while (true)
    {
        if (!modoNoturno)
        {
            for (int i = 0; i < 3; i++)
            {
                modo_verde(BUZZER_PIN); // envia o tom do sinal verde
            }

            for (int i = 0; i < 2; i++)
            {
                modo_amarelo(BUZZER_PIN); // envia o tom do sinal amarelo
            }

            for (int i = 0; i < 2; i++)
            {
                modo_vermelho(BUZZER_PIN); // envia o tom do sinal vermelho
            }
        }
        else
        {
            buzzer_noturno(BUZZER_PIN); // envia o tom do sinal amarelo no modo noturno
        }
    }
}

void vLedRGBTask()
{

    // inicializa os leds
    gpio_init(ledVerde);
    gpio_set_dir(ledVerde, GPIO_OUT);

    gpio_init(ledVermelho);
    gpio_set_dir(ledVermelho, GPIO_OUT);

    while (true)
    {
        if (!modoNoturno)
        {
            gpio_put(ledVerde, true);        // liga o verde
            vTaskDelay(pdMS_TO_TICKS(3000)); // espera a troca de sinal

            gpio_put(ledVermelho, true); // liga o vermelho para formar amarelo
            vTaskDelay(pdMS_TO_TICKS(2000));

            gpio_put(ledVerde, false); // desliga o verde para ficar apenas o vermelho

            vTaskDelay(pdMS_TO_TICKS(4000));

            gpio_put(ledVermelho, false); // desliga o vermelho
        }
        else
        {
            // verde + amarelo para formar amarelo
            gpio_put(ledVerde, true);
            gpio_put(ledVermelho, true);
            vTaskDelay(pdMS_TO_TICKS(950)); // espera a troca de sinal

            // desliga os leds
            gpio_put(ledVerde, false);
            gpio_put(ledVermelho, false);
            vTaskDelay(pdMS_TO_TICKS(950));
        }
    }
}

int main()
{
    stdio_init_all();

    xTaskCreate(vBotaoATask, "Botao Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vMatrizTask, "Matriz Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vDisplayTask, "Display Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vLedRGBTask, "Led RGB Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(vBuzzerTask, "Buzzer Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}
