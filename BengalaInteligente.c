#include <stdio.h>
#include "pico/stdlib.h"
#include "ws2818b.pio.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include <string.h>

// macros
#define LED_COUNT 25
#define LED_PIN 7
#define DEBOUNCE_DELAY_MS 200

#define LED_R 13
#define LED_G 11
#define LED_B 12

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define JOY_X 27
#define JOY_Y 26
#define BTN_JOY 22

#define BUZZER_A 21

#define LED_BRIGHTNESS 200 // define a intensidade dos leds

// protótipos
void init_hardware(void);
void pwm_init_gpio(uint pin);
void calibra_joystick();
int16_t ajustar_valor_joystick(int16_t raw, int16_t center);
static void gpio_irq_handler(uint gpio, uint32_t events);
void atualizarDisplay(void);

// variáveis globais
ssd1306_t ssd; // inicializa a estrutura do display
volatile uint32_t last_interrupt_A_time = 0;
volatile uint32_t last_interrupt_B_time = 0;
uint16_t center_x; // posição central do eixo X do joystick
uint16_t center_y; // posição central do eixo Y do joystick

PIO np_pio;
uint sm;
struct pixel_t
{
  uint8_t G, R, B;
};
typedef struct pixel_t npLED_t;
npLED_t leds[LED_COUNT];

// variável para controlar o debounce
absolute_time_t last_press_time;

void init_hardware(void)
{
  adc_init();           // Inicializa o ADC
  adc_gpio_init(JOY_X); // Configura ADC no pino X do joystick
  adc_gpio_init(JOY_Y); // Configura ADC no pino Y do joystick

  gpio_init(BTN_JOY);             // Inicializa botão do joystick
  gpio_set_dir(BTN_JOY, GPIO_IN); // Define como entrada
  gpio_pull_up(BTN_JOY);          // Habilita pull-up interno

  // configura o buzzer
  gpio_init(BUZZER_A);
  gpio_set_dir(BUZZER_A, GPIO_OUT);
  gpio_put(BUZZER_A, 0); // Inicialmente desligado

  // configura botao A na GPIO 5 com pull-up e interrupção na borda de descida
  gpio_init(5);
  gpio_set_dir(5, GPIO_IN);
  gpio_pull_up(5);

  // configura botão B na GPIO 6 com pull-up e interrupção na borda de descida
  gpio_init(6);
  gpio_set_dir(6, GPIO_IN);
  gpio_pull_up(6);

  // inicializando pwm para led e buzzer
  pwm_init_gpio(BUZZER_A);
  pwm_init_gpio(LED_R);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // pull up the data line
  gpio_pull_up(I2C_SCL);                                        // pull up the clock line
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // inicializa o display
  ssd1306_config(&ssd);                                         // configura o display
  ssd1306_send_data(&ssd);                                      // envia os dados para o display

  // limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  last_press_time = get_absolute_time(); // inicializa o tempo do último botão pressionado

  printf("RP2040 inicializado.\n");
}

/**
 * Função de interrupção para tratar os botões A (GPIO 5) e B (GPIO 6)
 * Implementa debouncing software usando a constante DEBOUNCE_DELAY_MS.
 */
void gpio_irq_handler(uint gpio, uint32_t events)
{
  uint32_t current_time = to_ms_since_boot(get_absolute_time());

  if (gpio == 5)
  {
    if (current_time - last_interrupt_A_time > DEBOUNCE_DELAY_MS)
    {
      last_interrupt_A_time = current_time;

      gpio_put(LED_B, 0);                // desliga o led azul
      gpio_put(LED_G, !gpio_get(LED_G)); // alterna o estado do LED verde
    }
  }
  else if (gpio == 6)
  {
    if (current_time - last_interrupt_B_time > DEBOUNCE_DELAY_MS)
    {
      last_interrupt_B_time = current_time;
      gpio_put(LED_G, 0);                // desliga o led verde
      gpio_put(LED_B, !gpio_get(LED_B)); // alterna o estado do LED azul
    }
  }
}

// inicializa um pino GPIO como saída PWM
void pwm_init_gpio(uint pin)
{
  gpio_set_function(pin, GPIO_FUNC_PWM);   // Configura o pino para função PWM
  uint slice = pwm_gpio_to_slice_num(pin); // Obtém o slice PWM correspondente
  pwm_set_wrap(slice, 4095);               // Define o valor máximo do PWM
  pwm_set_enabled(slice, true);            // Habilita o PWM
}

void desativar_buzzer()
{
  uint slice_num = pwm_gpio_to_slice_num(BUZZER_A);
  pwm_set_enabled(slice_num, false); // Desliga o PWM
}

void calibra_joystick()
{
  const int samples = 150;       // número de amostras para calibração
  uint32_t sum_x = 0, sum_y = 0; // variáveis para somar os valores lidos
  for (int i = 0; i < samples; i++)
  {
    adc_select_input(0);
    sum_x += adc_read(); // lê eixo X e soma
    adc_select_input(1);
    sum_y += adc_read(); // lê eixo Y e soma
    sleep_ms(10);
  }
  center_x = sum_x / samples; // calcula média para centro X
  center_y = sum_y / samples; // calcula média para centro Y
}

// ajusta o valor do joystick considerando a zona morta
int16_t ajustar_valor_joystick(int16_t raw, int16_t center)
{
  int16_t diff = raw - center;         // calcula a diferença em relação ao centro
  return (abs(diff) < 200) ? 0 : diff; // se dentro da zona morta, retorna 0
}

void atualizarDisplay()
{
  ssd1306_fill(&ssd, false); // Limpa o display

  // ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);       // desenha um retângulo
  ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 10); // desenha uma string
  ssd1306_send_data(&ssd);                            // atualiza o display

  sleep_ms(40);
}

int main()
{
  stdio_init_all();
  init_hardware();

  gpio_set_irq_enabled_with_callback(5, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  gpio_set_irq_enabled(6, GPIO_IRQ_EDGE_FALL, true);

  // loop principal (as ações dos botões são tratadas via IRQ)
  while (true)
  {
    adc_select_input(0); // seleciona o ADC para eixo X. O pino 26 como entrada analógica
    uint16_t x_raw = adc_read();
    adc_select_input(1); // seleciona o ADC para eixo Y. O pino 27 como entrada analógica
    uint16_t y_raw = adc_read();
    printf("Joystick: X=%d, Y=%d\n", x_raw, y_raw);

    int16_t x_adj = ajustar_valor_joystick(x_raw, center_x);
    int16_t y_adj = ajustar_valor_joystick(y_raw, center_y);
    pwm_set_gpio_level(LED_R, abs(y_adj) * 2);
    pwm_set_gpio_level(LED_R, abs(x_adj) * 2);

    if (x_adj > 200 || y_adj > 200)
    {
      if (x_adj > y_adj)
      {
        pwm_set_gpio_level(BUZZER_A, abs(x_adj) * 2);
      }
      else
      {
        pwm_set_gpio_level(BUZZER_A, abs(y_adj) * 2);
      }
    }

    atualizarDisplay();
    sleep_ms(100);
  }
}