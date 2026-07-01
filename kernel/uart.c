#include <stdint.h>

#define UART0 0x10000000L

extern void uart_putc(char);

/* Imprime número decimal na UART */
void uart_print_uint(uint64_t value)
{
    char buf[32];
    int i = 0;

    // Caso especial: zero direto
    if (value == 0)
    {
        uart_putc('0');
        return;
    }

    // Converte número para string (base 10)
    while (value > 0)
    {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    // Imprime na ordem correta (invertida pela conversão)
    while (i--)
        uart_putc(buf[i]);
}

void uart_putc(char c)
{
    // Escrita direta no registrador de transmissão da UART
    *(volatile uint8_t*)UART0 = c;
}

void uart_print(const char *s)
{
    // Imprime string até o caractere nulo
    while (*s)
        uart_putc(*s++);
}
