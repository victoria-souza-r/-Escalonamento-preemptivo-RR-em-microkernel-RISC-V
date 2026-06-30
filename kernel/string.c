#include <stdint.h> 
#include "string.h"

void *memset(void *s, int c, uint64_t n)
{
    uint8_t *p = (uint8_t *)s;

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

    while (n--)
    {
        *d++ = *s++;
    }

    return dest;
}

int strcmp(const char *a, const char *b)
{
    while (*a && (*a == *b))
    {
        a++;
        b++;
    }

    return (unsigned char)*a - (unsigned char)*b;
}

char *strcpy(char *dest, const char *src)
{
    char *inicio = dest;

    while ((*dest++ = *src++))
    {
    }

    return inicio;
}

uint64_t strlen(const char *s)
{
    uint64_t len = 0;

    while (*s++)
    {
        len++;
    }

    return len;
}
