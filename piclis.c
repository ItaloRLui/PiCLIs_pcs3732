#include "uart.h"
#include "gpio.h"
#include <stdbool.h>
#include <stdint.h>
void delay(unsigned);

/*
 * Sinais reconhecidos pelo PiCLIs
 */
#define SIG_HUP            0x01
#define SIG_INT            0x02
#define SIG_QUIT           0x03
#define SIG_ILL            0x04
#define SIG_TRAP           0x05
#define SIG_ABRT           0x06
#define SIG_KILL           0x09
#define SIG_SEGV           0x0b
#define SIG_SYS            0x0a
#define SIG_TERM           0x0f
#define SIG_STOP           0x11

/*
 * Instrução usada como trap (breakpoint)
 */
#define TRAP_INST          0xefaaaaaa

#define MEMORY(X)          *((uint32_t*)(X))

/*
 * Símbolos declarados pelo linker
 */
extern uint8_t *stack_svr, *load_addr;

/*
 * Valor dos registradores do usuário.
 */
#define NUM_REGS           42
uint32_t user_regs[NUM_REGS] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // r0-r12
   (uint32_t)&stack_svr,                      // sp
   0,                                         // lr
   (uint32_t)&load_addr,                      // pc
   0, 0, 0, 0, 0, 0, 0, 0,                    // f0-f7 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
   0,                                         // fps
   0x10                                       // cpsr
};
uint8_t user_status = SIG_TRAP;

#define PC                 (user_regs[15])
#define CPSR               (user_regs[41])

// 0-15  --- r0-r15
// 16-23 --- f0-f7
// 24    --- FPS (floating point status) na escrita
// 25    --- processor status na escrita
// 26-39 --- 
// 40    --- fps
// 41    --- cpsr

/*
 * Breakpoints.
 */
#define MAX_BKPTS          16
typedef struct bkpt_s {
   uint32_t addr;
   uint32_t cont;
} bkpt_t;
bkpt_t bkpts[MAX_BKPTS] = { 0 };

/*
 * Funções para manipulação de caracteres hexadecimais.
 */
char hex_to_char(uint8_t n) {
   if(n < 10) return '0' + n;
   if(n < 16) return 'a' + n - 10;
   return '0';
}

int char_to_hex(char c) {
   if((c >= '0') && (c <= '9')) return c - '0';
   if((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
   if((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
   return 0;
}

/**
 * Recebe um caractere (byte hexadecimal) e faz o LED verde (GPIO 47) piscá-lo em código morse.
 * @param c Valor a codificar (8 bits).
 */
void char_to_morse(char c) {
   if (((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))) {
      gpio_put(47, 1);
      switch (c) {
      case 'a':
      case 'A':
         delay(1000000); // Ponto (1 unidade)
         gpio_toggle(47);
         delay(1000000); // Espaço entre partes do mesmo caractere (1 unidade)
         gpio_toggle(47);
         delay(3000000); // Traço (3 unidades)
         break;
      case 'b':
      case 'B':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 'c':
      case 'C':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 'd':
      case 'D':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 'e':
      case 'E':
         delay(1000000); // Ponto
         break;
      case 'f':
      case 'F':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 'g':
      case 'G':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 'h':
      case 'H':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 'i':
      case 'I':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 'j':
      case 'J':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case 'k':
      case 'K':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case 'l':
      case 'L':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 'm':
      case 'M':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case 'n':
      case 'N':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 'o':
      case 'O':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case 'p':
      case 'P':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 'q':
      case 'Q':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case 'r':
      case 'R':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 's':
      case 'S':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case 't':
      case 'T':
         delay(3000000); // Traço
         break;
      case 'u':
      case 'U':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case 'v':
      case 'V':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case 'w':
      case 'W':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case 'x':
      case 'X':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case 'y':
      case 'Y':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case 'z':
      case 'Z':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case '1':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case '2':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case '3':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case '4':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      case '5':
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case '6':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case '7':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case '8':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case '9':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(1000000); // Ponto
         break;
      case '0':
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         gpio_toggle(47);
         delay(1000000); // Espaço
         gpio_toggle(47);
         delay(3000000); // Traço
         break;
      default:
         break;
      }
   }
}

/**
 * Envia um byte em hexadecimal pela uart.
 * @param v Valor a enviar (8 bits).
 * @return Checksum dos caracteres enviados.
 */
uint8_t sendbyte(uint8_t v) {
   uint8_t chk = 0, c;
   c = hex_to_char(v >> 4);
   uart_putc(c);  chk += c;
   c = hex_to_char(v & 0x0f);
   uart_putc(c);  chk += c;
   return chk;
}

/**
 * Recebe um byte (dois caracteres hexadecimais) pela uart.
 * @return Byte recebido.
 */
uint8_t readbyte(void) {
   int res = char_to_hex(uart_getc());
   return (res << 4) | char_to_hex(uart_getc());
}

/**
 * Envia uma mensagem completa contendo os dados de uma área de memória.
 * @param a Endereço da área de memória.
 * @param s Quantidade de bytes a enviar.
 */
void sendbytes(uint8_t *a, uint32_t s) {
   uint8_t chk = 0;

   while(s) {
      chk += sendbyte(*a);
      a++;
      s--;
      if (s % 15 == 0) uart_puts("\r\n");
   }
   // uart_putc('#');
   // sendbyte(chk);
}

/**
 * Compara bytes de uma área da memória com uma palavra de dados fornecida
 * @param search_word Palavra de dados
 * @param a Endereço da área de memória.
 * @param s Tamanho da área de memória.
 * @return Número de ocorrências da palavra de dados na região da memória.
 */
uint8_t compbytes(uint32_t search_word, uint8_t *a, uint32_t s) {
   uint8_t times = 0;
   unsigned char buffer[4];

   while (s) {
      uint32_t variable;
      buffer[0] = ((*a << 3)| (*(a+1) << 2)| (*(a+2) << 1)| (*(a+3)));
      buffer[1] = ((*(a+4) << 3)| (*(a+5) << 2)| (*(a+6) << 1)| (*(a+7)));
      buffer[2] = ((*(a+8) << 3)| (*(a+9) << 2)| (*(a+10) << 1)| (*(a+11)));
      buffer[3] = ((*(a+12) << 3)| (*(a+13) << 2)| (*(a+14) << 1)| (*(a+15)));
      variable = ((buffer[0] << 24)| (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
      if (variable == search_word) {
         times++;
      }
      a++;
      s--;
   }
   
   return times;
}

/**
 * Recebe uma quantidade de bytes (caracteres hexadecimais) pela uart.
 * @param a Endereço inicial para salvar os dados recebidos.
 * @param s Quantidade de bytes a receber.
 */
void readbytes(uint8_t *a, uint32_t s) {
   while(s) {
      *a++ = readbyte();
      s--;
   }
}

/**
 * Recebe um inteiro de 32 bits (de um a oito caracteres hexadecimais, 
 * delimitados por um caractere específico).
 * @param t Caractere delimitador.
 * @return Valor uint32_t recebido.
 */
uint32_t readword(char t) {
   uint32_t res = 0;
   uint8_t c;
   for(;;) {
      c = uart_getc();
      if(c == t) return res;
      res = (res << 4) | char_to_hex(c);
   }
}

/**
 * Troca o endianess de big para little ou vice-versa.
 */
uint32_t endian_change(uint32_t v) {
   uint32_t res = 0;
   res |= (v & 0xff) << 24;
   res |= (v & 0xff00) << 8;
   res |= (v & 0xff0000) >> 8;
   res |= (v & 0xff000000) >> 24;
   return res;
}

/**
 * Recebe e descarta caracteres pela uart até a recepção do 
 * caractere especificado.
 * @param t Caractere a receber.
 */
void skip(char t) {
   for(;;) {
      uint8_t c = uart_getc();
      if(c == t) return;
   }
}

/**
 * Aguarda a conclusão de uma mensagem e envia um ack ('+').
 */
void ack(void) {
   skip('#');
   // uart_getc();                     // ignora checksum
   // uart_getc();
   uart_putc('+');                  // responde com um acknowledge
}

/**
 * Acrescenta um novo breakpoint na lista.
 * @param addr Endereço da memória para o breakpoint.
 * @return false se não houver espaço para um novo breakpoint.
 */
bool bkpt_add(uint32_t addr) {
   int j = 0;
   if(addr == 0) return false;
   for(int i=1; i<MAX_BKPTS; i++) {
      if(bkpts[i].addr == addr) return true;           // breakpoint redefinido
      if(bkpts[i].addr == 0) j = i;                    // posição vaga
   }

   if(j == 0) return false;                            // nenhuma posição vaga
   bkpts[j].addr = addr;
   return true;
}

/**
 * Remove um breakpoint da lista.
 * @param addr Endereço de memória do breakpoint.
 * @return false se o breakpoint não foi encontrado.
 */
bool bkpt_remove(uint32_t addr) {
   for(int i=1; i<MAX_BKPTS; i++) {
      if(bkpts[i].addr == addr) {
         bkpts[i].addr = 0;
         return true;
      }
   }
   return false;
}

/**
 * Limpa a memória de instruções trap.
 */
void bkpt_restore_contents(void) {
   for(int i=0; i<MAX_BKPTS; i++) {
      uint32_t addr = bkpts[i].addr;
      if(addr == 0) continue;
      MEMORY(addr) = bkpts[i].cont;
   }
}

/**
 * Coloca traps na memória nas posições dos breakpoints ativos.
 */
void bkpt_activate(void) {
   for(int i=0; i<MAX_BKPTS; i++) {
      uint32_t addr = bkpts[i].addr;
      if(addr == 0) continue;
      bkpts[i].cont = MEMORY(addr);
      MEMORY(addr) = TRAP_INST;
   }
}

/**
 * Ponto de entrada do loop de processamento de mensagens do stub.
 */
void piclis_main(int sig) {
   static uint32_t a, s;
   static uint8_t chk;
   static uint8_t c;

   /*
    * Limpa brakepoints da memória
    */
   uart_break_disable();
   bkpt_restore_contents();
   bkpts[0].addr = 0;

   /*
    * Envia status ao depurador.
    */
   user_status = sig;
   // uart_puts("$S");
   // chk = 'S' + sendbyte(user_status);
   // uart_putc('#');
   // sendbyte(chk);

   /*
    * Espera uma mensagem.
    */
retry:
   uart_puts("\r\n> ");

   /*
    * Identifica a mensagem
    */
   c = uart_getc();
   switch(c) {
      case '?':
            goto trata_status;
      case '$':
         c = uart_getc();
         if (c == 'p') {
            c = uart_getc();
            if (c == 'B') {
               c = uart_getc();
               if (c == 'I') {
                  c = uart_getc();
                  if (c == 'N') goto trata_dectobin;
               }
               break;
            }
            if (c == 'C') {
               c = uart_getc();
               if (c == 'H') {
                  c = uart_getc();
                  if (c == 'K') goto trata_checksum;
               }
               break;
            }
            if (c == 'S') {
               c = uart_getc();
               if (c == 'C') {
                  c = uart_getc();
                  if (c == 'H') goto trata_search;
               }
               break;
            }
            if (c == 'E') {
               c = uart_getc();
               if(c == 'C') {
                  c = uart_getc();
                  if(c == 'H') {
                     c = uart_getc();
                     if(c == 'O') goto trata_echo;
                  }
                  break;
               }
               break;
            }
            if (c == 'M') {
               c = uart_getc();
               if (c == 'O') {
                  c = uart_getc();
                  if (c == 'R') {
                     c = uart_getc();
                     if (c == 'S') {
                        c = uart_getc();
                        if (c == 'E') goto trata_morse;
                     }
                     break;
                  }
                  break;
               }
               break;
            }
            break;
         }
      case 'g':
         goto trata_g;
      case 'G':
         goto trata_G;
      case 'P':
         goto trata_P;
      case 'm':
         goto trata_m;
      case 'M':
         goto trata_M;
      case 'c':
         ack();
         goto executa;
      case 's':
         goto trata_s;
      case 'Z':
         c = uart_getc();
         if(c == '0') goto trata_Z0;
         break;
      case 'z':
         c = uart_getc();
         if(c == '0') goto trata_z0;
         break;
      case 'D':
      case 'k':
         ack();
         goto envia_ok;
      case '\r':
         goto retry;
   }
   for (;;) { // Se algum comando for escrito errado
      if (c == '\r') goto retry;
      c = uart_getc();
   }

   /*
    * Comando não reconhecido
    */
envia_nulo:
   uart_puts("$#00");
   goto retry;

envia_ok:
   uart_puts("$OK#9a");
   goto retry;

envia_erro:
   uart_puts("$E01#a5");
   goto retry;

executa:
   bkpt_activate();
   uart_break_enable();
   asm volatile ("b switch_back");
   goto retry;

trata_morse:
   /*
   * Pisca uma palavra formada por caracteres alfanuméricos em código morse pelo LED verde da placa. 
   * A palavra pode ter até 99 caracteres.
   * Formato do comando: $pMORSE <palavra>
   */
   skip(' ');
   char palavra[100];
   int letra = 0;
   uint8_t caractere = uart_getc();
   char caractere_char = (char) caractere;
   while (caractere_char != '\r' && letra < 99) {
      palavra[letra] = caractere_char;
      letra++;
      caractere = uart_getc();
      caractere_char = (char) caractere;
   }
   uart_puts("+");

   for (int i = 0; i < letra; i++) {
      char_to_morse(palavra[i]);
      uart_putc(palavra[i]);
      gpio_put(47, 0); // Desliga
      delay(3000000); // Espaço entre caracteres (3 unidades)
   }
   goto retry;

trata_search:
   /*
   * Conta o número de ocorrências de uma palavra de dados na memória (até 4 caracteres)
   */
   skip(' ');
   uint8_t first = char_to_hex(uart_getc());
   uint8_t second = char_to_hex(uart_getc());
   uint8_t third = char_to_hex(uart_getc());
   uint32_t search_word = (first << 12) | (second << 8) | (third << 4) | char_to_hex(uart_getc());     // palavra de dados
   skip(' ');
   uint32_t start_address_search = readword(' ');                   // endereço inicial
   uint32_t search_size = readword('\r');                   // tamanho da área de dados

   uint8_t times_search = compbytes(search_word, (uint8_t *) start_address_search, search_size);

   uart_puts("A palavra ");
   uart_puts(search_word);
   uart_puts(" aparece ");
   uart_putc(hex_to_char(times_search));
   uart_puts(" vezes na area procurada.");
   goto retry;

trata_checksum:
   /*
   * Faz o checksum de uma área de memória.
   */
  sendbytes((uint8_t*) 400, 32);
  goto retry;

trata_echo:
   /*
   * Envia uma mensagem para a placa, e retorna ela pela UART, para garantir seu funcionamento.
   * A mensagem pode ter até 99 caracteres.
   * Formato do comando $pECHO <palavra>
   */
   skip(' ');
   char palavra_echo[100];
   int letra_echo = 0;
   uint8_t caractere_echo = uart_getc();
   char caractere_char_echo = (char) caractere_echo;
   while (caractere_char_echo != '\r' && letra_echo < 99) {
      palavra_echo[letra_echo] = caractere_echo;
      letra_echo++;
      caractere_echo = uart_getc();
      caractere_char_echo = (char) caractere_echo;
   }
   uart_puts("+");

   for (int j = 0; j < letra_echo; j++) {
      uart_putc(palavra_echo[j]);
   }
   uart_puts("\r\n");
   goto retry;

trata_dectobin:
   /*
   * Converte um número fornecido de decimal para binário, tratando casos negativos com complemento de dois, e envia-o serialmente.
   * O limite do valor decimal é de 2147483647
   * Formato do comando $pBIN <número decimal>
   */
        skip(' ');

        int numero_decimal = 0;
        int is_negative = 0;
        uint8_t char_decimal = uart_getc();

        if (char_decimal == '-') {
            is_negative = 1;
            char_decimal = uart_getc();
        }

        while (char_decimal != '\r') {
            int algarismo_decimal = char_to_hex((char)char_decimal);
            numero_decimal = numero_decimal * 10 + algarismo_decimal;
            char_decimal = uart_getc();
        }

        if (is_negative) {
            numero_decimal = -numero_decimal;
        }

        uart_puts(">");

        uint32_t numero_binario = (uint32_t)numero_decimal;
        int algarismos_binarios = 32;
        char bin_str[33];
        bin_str[32] = '\0';

        for (int i = 31; i >= 0; i--) {
            bin_str[i] = (numero_binario & 1) ? '1' : '0';
            numero_binario >>= 1;
        }

        uart_puts(bin_str);
        if(is_negative){
        	uart_puts(" (Complemento de Dois)");
        }
        uart_puts("\r\n");
        
   goto retry;

trata_g:
   /*
    * Envia todos os registradores.
    */
   ack();
   sendbytes((uint8_t*)user_regs, sizeof(user_regs));
   goto retry;

trata_G:
   /*
    * Altera todos os registradores.
    */
   readbytes((uint8_t*)user_regs, sizeof(user_regs));
   ack();
   goto envia_ok;

trata_P:
   /*
    * Altera um dos registradores.
    */
   a = readword('=');                   // índice do registrador
   s = readword('#');
   uart_getc();
   uart_getc();
   uart_putc('+');
   if(a <= NUM_REGS) {
      user_regs[a] = endian_change(s);
   }
   goto envia_ok;

trata_m:
   /*
    * Lê memória.
    */
   skip(' ');
   a = readword(' ');                   // endereço inicial
   s = readword('\r');                   // tamanho

   sendbytes((uint8_t*)a, s);
   goto retry;

trata_M:
   /*
    * Escreve memória.
    */
   skip(' ');
   a = readword(' ');                   // endereço inicial
   s = readword('\r');                   // tamanho

   readbytes((uint8_t*)a, s);
   goto retry;

trata_status:
   /*
    * Envia o último sinal.
    */
   ack();
   uart_puts("$S");
   chk = 'S' + sendbyte(user_status);
   uart_putc('#');
   sendbyte(chk);
   goto retry;

trata_s:
   /*
    * Executa a próxima instrução.
    * Introduz um trap na instrução seguinte.
    * (não funciona se for um salto...)
    */
   ack();
   bkpts[0].addr = PC + 4;
   goto executa;

trata_Z0:
   /*
    * Inclui um breakpoint de software.
    */
   uart_getc();        // ','
   a = readword(',');
   ack();
   if(bkpt_add(a)) goto envia_ok;
   goto envia_erro;

trata_z0:
   /*
    * Remove um breakpoint de software.
    */
   uart_getc();        // ','
   a = readword(',');
   ack();
   bkpt_remove(a);
   goto envia_ok;
}

/**
 * Inicialização em C.
 */
void main(void) {
   uart_init();
   gpio_init(47, 1);

   delay(100);
   uart_puts("PiCLIs - Raspberry Pi CLI!\r\n");
   uart_puts("Por Henrique Murakami, Italo Lui e Rafael Tamasi\r\n");
   asm volatile (
      "mov r0, #0x05 \n\t"
      "b piclis_main \n\t"
   );
}

