#define GPIO_FUNC_INPUT       0
#define GPIO_FUNC_OUTPUT      1
#define GPIO_FUNC_ALT5        2
#define GPIO_FUNC_ALT4        3
#define GPIO_FUNC_ALT0        4
#define GPIO_FUNC_ALT1        5
#define GPIO_FUNC_ALT2        6
#define GPIO_FUNC_ALT3        7

#define GPIO_PULL_NONE        0
#define GPIO_PULL_DOWN        1
#define GPIO_PULL_UP          2

#define ARM_MODE_USER         0b10000
#define ARM_MODE_FIQ          0b10001
#define ARM_MODE_IRQ          0b10010
#define ARM_MODE_SVR          0b10011
#define ARM_MODE_MONITOR      0b10110
#define ARM_MODE_ABORT        0b10111
#define ARM_MODE_UNDEF        0b11011
#define ARM_MODE_HYPER        0b11010
#define ARM_MODE_SYSTEM       0b11111

/***
 * gpio_init 
 * -- parâmtros: r0 (índice do GPIO, de 0 a 53)
 * --            r1 (função desejada GPIO_FUNC_...)
 * -- retorno -
 * Configura um GPIO antes do uso.
 ***/
void gpio_init(unsigned, unsigned);

/*** 
 * gpio_put 
 * -- parâmtros: r0 (índice do GPIO, de 0 a 53)
 * --            r1 (valor a escrever, 0 ou 1)
 * -- retorno -
 * Altera o valor de um GPIO configurado como saída
 ***/
void gpio_put(unsigned, unsigned);

/***
 * gpio_get 
 * -- parâmtros: r0 (índice do GPIO, de 0 a 53)
 * -- retorno r0 (valor atual do GPIO)
 * Lê o estado atual de um GPIO
 ***/
unsigned gpio_get(unsigned);

/***
 * gpio_toggle 
 * -- parâmtros: r0 (índice do GPIO, de 0 a 53)
 * -- retorno -
 * Alterna o estado de um GPIO configurado como saída.
 ***/
void gpio_toggle(unsigned);

/***
 * gpio_set_pulls 
 * -- parâmtros: r0 (índice do GPIO, de 0 a 53)
 * --            r1 (configuração de pull-up (GPIO_PULL_...))
 * -- retorno -
 * Configura os resistores internos conectados a um GPIO.
 ***/
void gpio_set_pulls(unsigned, unsigned);