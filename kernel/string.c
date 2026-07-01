#include <stdint.h>
#include "string.h"

void *memset(void *s, int c, uint64_t n)
{
    uint8_t *p = (uint8_t *)s;

    // Preenche um bloco de memória com um valor constante
    while (n--)
    {
        *p++ = (uint8_t)c;
    }

    return s;
}

void *memcpy(void *dest, const void *src, uint64_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    // Copia n bytes de uma região para outra (sem sobreposição segura)
    while (n--)
    {
        *d++ = *s++;
    }

    return dest;
}

int strcmp(const char *a, const char *b)
{
    // Compara duas strings até encontrar diferença ou fim
    while (*a && (*a == *b))
    {
        a++;
        b++;
    }

    // Retorna diferença lexicográfica
    return (unsigned char)*a - (unsigned char)*b;
}

char *strcpy(char *dest, const char *src)
{
    char *inicio = dest;

    // Copia string incluindo o '\0'
    while ((*dest++ = *src++))
    {
    }

    return inicio;
}

char *strncpy(char *dest, const char *src, uint64_t n)
{
    uint64_t i;

    // Copia até n caracteres ou até o fim da string
    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }

    // Completa com '\0' se sobrar espaço
    for (; i < n; i++)
    {
        dest[i] = '\0';
    }

    return dest;
}

uint64_t strlen(const char *s)
{
    uint64_t len = 0;

    // Conta caracteres até encontrar o terminador
    while (*s++)
    {
        len++;
    }

    return len;
}
