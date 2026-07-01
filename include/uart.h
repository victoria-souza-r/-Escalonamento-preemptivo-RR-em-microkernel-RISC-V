#pragma once
#include <stdint.h>

// Interface para comunicação pela UART
void uart_putc(char c);
void uart_print(const char *s);
void uart_print_uint(uint64_t value);
