#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"
#include "ws2818b.pio.h"
#include "inc/ssd1306.h"

#define LED_COUNT 25
#define LED_PIN 7
#define GREEN_PIN 11
#define BLUE_PIN 12

// Variáveis de Estado dos LEDs
volatile bool green_value = 0;
volatile bool blue_value = 0;

// Flag
volatile char c = '~';
volatile bool new_data = false;

// Variáveis do I2C
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// Definição da estrutura do pixel
struct pixel_t {
    uint8_t G, R, B;
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;

npLED_t leds[LED_COUNT];
PIO np_pio;
uint sm;

volatile int current_digit = 0;
absolute_time_t last_interrupt_time = 0;

// Protótipos das funções
void npDisplayDigit(int digit);

// Matrizes para cada dígito 
const uint8_t digits[11][5][5][3] = {
    // Dígito 0
    {
        {{0, 0, 0}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 0}}, 
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},    
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},    
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},    
        {{0, 0, 0}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 0}}  
    },
    // Dígito 1
    {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 110}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 110}, {0, 0, 110}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 110}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 110}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}}
    },
    // Dígito 2
    {
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}},
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}} 
    },
    // Dígito 3
    {
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 0}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}},   
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}} 
    },
    // Dígito 4
    {
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}}
    },
    // Dígito 5
    {
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}},
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}} 
    },
    // Dígito 6
    {
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 0}},   
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}},
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}} 
    },
    // Dígito 7
    {
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 110}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
    },
    // Dígito 8
    {
        {{0, 0, 0}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 0}},
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 0}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 0}},
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 0}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 0}}
    },
    // Dígito 9
    {
        {{0, 0, 0}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 0}},      
        {{0, 0, 110}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 110}},
        {{0, 0, 0}, {0, 0, 110}, {0, 0, 110}, {0, 0, 110}, {0, 0, 0}}       
    },
    // Inicializar zerado
    {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},      
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}       
    }

};

// Função de callback para interrupções
void gpio_callback(uint gpio, uint32_t events) {
    absolute_time_t now = get_absolute_time();
    int64_t diff = absolute_time_diff_us(last_interrupt_time, now);

    if (diff < 250000) return; // Debounce de 250ms = 1/4 de segundo
    last_interrupt_time = now;

    if (gpio == 5) {
        green_value = !green_value;
        gpio_put(GREEN_PIN, green_value);
        green_value ? printf("LED Verde ligado\n") : printf("LED Verde desligado\n");
        new_data = true;
        if(green_value){c = '!';}else{c = '@';}
    } else if (gpio == 6) {
        blue_value = !blue_value;
        gpio_put(BLUE_PIN, blue_value);
        blue_value ? printf("LED Azul ligado\n") : printf("LED Azul desligado\n");
        new_data = true;
        if(blue_value){c = '#';}else{c = '$';}
    }
}

void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

void npClear() {
   current_digit = 10;
   npDisplayDigit(current_digit);
}


// Inicialização da matriz de LEDs
void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, true);
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
    npClear();
}


void npWrite() {
    for (uint i = 0; i < LED_COUNT; i++) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100);
}

int getIndex(int x, int y) {
    if (y % 2 == 0) {
        return 24 - (y * 5 + x);
    } else {
        return 24 - (y * 5 + (4 - x));
    }
}

// Função auxiliar para processar o comando e atualizar os displays
void process_command(char c, int digit, char *line1, char *line2, uint8_t *ssd, struct render_area *frame_area) {
    if (strchr("!@#$", c) != NULL) {
    sleep_ms(5);
    } else {
        printf("O comando foi %c\n", c);
    }   

    current_digit = digit;
    npDisplayDigit(current_digit);

    // Atualiza o OLED
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, frame_area);
    ssd1306_draw_string(ssd, 5, 0, line1);
    ssd1306_draw_string(ssd, 5, 8, line2);
    render_on_display(ssd, frame_area);
}

int main() {
    stdio_init_all();
    npInit(LED_PIN);

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();

    // Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // zera o display inteiro
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);


    // Configuração dos botões
    gpio_init(5);
    gpio_set_dir(5, GPIO_IN);
    gpio_pull_up(5);

    gpio_init(6);
    gpio_set_dir(6, GPIO_IN);
    gpio_pull_up(6);

    //Inicialização do RGB
    gpio_init(BLUE_PIN);
    gpio_set_dir(BLUE_PIN, GPIO_OUT);
    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);

    // Configuração das interrupções
    gpio_set_irq_enabled(5, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(6, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_callback(gpio_callback);
    irq_set_enabled(IO_IRQ_BANK0, true);

    while (true) {
        sleep_ms(50); //Estabilizar o sistema

        // Verifica entrada do terminal
        int input = getchar_timeout_us(0); // Mantém como int para verificação de erro
        if (input != PICO_ERROR_TIMEOUT || new_data) {
            if (!new_data){c = (char)input; // Converte para char após verificar que é válido
            }
            switch(c) {
                // Comandos para os números
                case '0': process_command(c, 0, "numero", "  00  ", ssd, &frame_area); break;
                case '1': process_command(c, 1, "numero", "  01  ", ssd, &frame_area); break;
                case '2': process_command(c, 2, "numero", "  02  ", ssd, &frame_area); break;
                case '3': process_command(c, 3, "numero", "  03  ", ssd, &frame_area); break;
                case '4': process_command(c, 4, "numero", "  04  ", ssd, &frame_area); break;
                case '5': process_command(c, 5, "numero", "  05  ", ssd, &frame_area); break;
                case '6': process_command(c, 6, "numero", "  06  ", ssd, &frame_area); break;
                case '7': process_command(c, 7, "numero", "  07  ", ssd, &frame_area); break;
                case '8': process_command(c, 8, "numero", "  08  ", ssd, &frame_area); break;
                case '9': process_command(c, 9, "numero", "  09  ", ssd, &frame_area); break;

                // Comandos para as letras
                case 'A': process_command(c, 10, "letra", "  A  ", ssd, &frame_area); break;
                case 'B': process_command(c, 10, "letra", "  B  ", ssd, &frame_area); break;
                case 'C': process_command(c, 10, "letra", "  C  ", ssd, &frame_area); break;
                case 'D': process_command(c, 10, "letra", "  D  ", ssd, &frame_area); break;
                case 'E': process_command(c, 10, "letra", "  E  ", ssd, &frame_area); break;
                case 'F': process_command(c, 10, "letra", "  F  ", ssd, &frame_area); break;
                case 'G': process_command(c, 10, "letra", "  G  ", ssd, &frame_area); break;
                case 'H': process_command(c, 10, "letra", "  H  ", ssd, &frame_area); break;
                case 'I': process_command(c, 10, "letra", "  I  ", ssd, &frame_area); break;
                case 'J': process_command(c, 10, "letra", "  J  ", ssd, &frame_area); break;
                case 'K': process_command(c, 10, "letra", "  K  ", ssd, &frame_area); break;
                case 'L': process_command(c, 10, "letra", "  L  ", ssd, &frame_area); break;
                case 'M': process_command(c, 10, "letra", "  M  ", ssd, &frame_area); break;
                case 'N': process_command(c, 10, "letra", "  N  ", ssd, &frame_area); break;
                case 'O': process_command(c, 10, "letra", "  O  ", ssd, &frame_area); break;
                case 'P': process_command(c, 10, "letra", "  P  ", ssd, &frame_area); break;
                case 'Q': process_command(c, 10, "letra", "  Q  ", ssd, &frame_area); break;
                case 'R': process_command(c, 10, "letra", "  R  ", ssd, &frame_area); break;
                case 'S': process_command(c, 10, "letra", "  S  ", ssd, &frame_area); break;
                case 'T': process_command(c, 10, "letra", "  T  ", ssd, &frame_area); break;
                case 'U': process_command(c, 10, "letra", "  U  ", ssd, &frame_area); break;
                case 'V': process_command(c, 10, "letra", "  V  ", ssd, &frame_area); break;
                case 'W': process_command(c, 10, "letra", "  W  ", ssd, &frame_area); break;
                case 'X': process_command(c, 10, "letra", "  X  ", ssd, &frame_area); break;
                case 'Y': process_command(c, 10, "letra", "  Y  ", ssd, &frame_area); break;
                case 'Z': process_command(c, 10, "letra", "  Z  ", ssd, &frame_area); break;
                case 'a': process_command(c, 10, "letra", "  a  ", ssd, &frame_area); break;
                case 'b': process_command(c, 10, "letra", "  b  ", ssd, &frame_area); break;
                case 'c': process_command(c, 10, "letra", "  c  ", ssd, &frame_area); break;
                case 'd': process_command(c, 10, "letra", "  d  ", ssd, &frame_area); break;
                case 'e': process_command(c, 10, "letra", "  e  ", ssd, &frame_area); break;
                case 'f': process_command(c, 10, "letra", "  f  ", ssd, &frame_area); break;
                case 'g': process_command(c, 10, "letra", "  g  ", ssd, &frame_area); break;
                case 'h': process_command(c, 10, "letra", "  h  ", ssd, &frame_area); break;
                case 'i': process_command(c, 10, "letra", "  i  ", ssd, &frame_area); break;
                case 'j': process_command(c, 10, "letra", "  j  ", ssd, &frame_area); break;
                case 'k': process_command(c, 10, "letra", "  k  ", ssd, &frame_area); break;
                case 'l': process_command(c, 10, "letra", "  l  ", ssd, &frame_area); break;
                case 'm': process_command(c, 10, "letra", "  m  ", ssd, &frame_area); break;
                case 'n': process_command(c, 10, "letra", "  n  ", ssd, &frame_area); break;
                case 'o': process_command(c, 10, "letra", "  o  ", ssd, &frame_area); break;
                case 'p': process_command(c, 10, "letra", "  p  ", ssd, &frame_area); break;
                case 'q': process_command(c, 10, "letra", "  q  ", ssd, &frame_area); break;
                case 'r': process_command(c, 10, "letra", "  r  ", ssd, &frame_area); break;
                case 's': process_command(c, 10, "letra", "  s  ", ssd, &frame_area); break;
                case 't': process_command(c, 10, "letra", "  t  ", ssd, &frame_area); break;
                case 'u': process_command(c, 10, "letra", "  u  ", ssd, &frame_area); break;
                case 'v': process_command(c, 10, "letra", "  v  ", ssd, &frame_area); break;
                case 'w': process_command(c, 10, "letra", "  w  ", ssd, &frame_area); break;
                case 'x': process_command(c, 10, "letra", "  x  ", ssd, &frame_area); break;
                case 'y': process_command(c, 10, "letra", "  y  ", ssd, &frame_area); break;
                case 'z': process_command(c, 10, "letra", "  z  ", ssd, &frame_area); break;

                // Comandos para os LEDs
                case '!': process_command(c, 10, "led verde", "  ligado  ", ssd, &frame_area); break;
                case '@': process_command(c, 10, "led verde", "  desligado  ", ssd, &frame_area); break;
                case '#': process_command(c, 10, "led azul", "  ligado  ", ssd, &frame_area); break;
                case '$': process_command(c, 10, "led azul", "  desligado  ", ssd, &frame_area); break;
                
                // Comandos para limpar a matriz
                case '~': break;
                default: printf("Comando desconhecido: %c\n", c); break;
            }
            new_data = false;
        }
    }

}

void npDisplayDigit(int digit) {
    for (int coluna = 0; coluna < 5; coluna++) {
        for (int linha = 0; linha < 5; linha++) {
            int posicao = getIndex(linha, coluna);
            npSetLED(
                posicao,
                digits[digit][coluna][linha][0],  // R
                digits[digit][coluna][linha][1],  // G
                digits[digit][coluna][linha][2]   // B
            );
        }
    }
    npWrite();
}

