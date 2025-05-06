# Semáforo inteligente
Este projeto consiste na implementação de um sistema de semáforo inteligente utilizando a placa BitDog Lab com o microcontrolador RP2040, programado com FreeRTOS. O objetivo é simular um semáforo com dois modos de operação, normal e noturno, incluindo sinalização sonora acessível para pessoas com deficiência visual.

## Funcionalidades
- Modo Normal:
  - LED Verde aceso: 1 beep curto por segundo.
  - LED Amarelo aceso: beep rápido intermitente.
  - LED Vermelho aceso: tom contínuo curto (0,5s ligado / 1,5s desligado).
- Modo Noturno:
  - Apenas o LED Amarelo pisca lentamente.
  - Buzzer emite um beep lento a cada 2 segundos.

- Alternância de modo:
  - Pressionar o botão A alterna entre o modo Normal e Noturno, controlado por uma flag global modificada por uma tarefa.
- Acessibilidade sonora:
  - Cada cor do semáforo possui um padrão de som distinto, permitindo que pessoas cegas identifiquem o estado do semáforo.
 
## Como Usar
- Conecte a placa BitDog Lab com o RP2040 ao seu computador.
- Faça o upload do código (disponível no repositório) para a placa.
- O sistema iniciará no Modo Normal.
- Pressione o botão A para alternar entre os modos de operação.
- Observe os LEDs e o buzzer indicando o estado do semáforo.

## Componentes Utilizados:
- Placa BitDog Lab com RP2040
- Matriz de LEDs
- LED RGB
- Display
- Buzzer
- Botão A
- Sistema operacional FreeRTOS

## Autor
### Matheus Nepomuceno Souza
