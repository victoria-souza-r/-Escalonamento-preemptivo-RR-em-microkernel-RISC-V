#pragma once
#include <stdint.h>

/* Inicializa o heap */
void memory_init(void);

/* Alocação simples */
void *kmalloc(uint64_t size);

/* Liberação */
void kfree(void *ptr);

/* Informações */
uint64_t memory_used(void);
uint64_t memory_free(void);
