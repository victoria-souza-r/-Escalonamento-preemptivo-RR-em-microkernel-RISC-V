#include <stdint.h> // para o compilador entender uint8_t e uint64_t
#include "memory.h"

//* Configuração do heap   */

// Região fixa de heap dentro do espaço de memória
#define HEAP_START 0x80400000UL
#define HEAP_SIZE  (8 * 1024 * 1024)   // 8 MB

static uint8_t *heap_base = (uint8_t*)HEAP_START;
static uint8_t *heap_end  = (uint8_t*)(HEAP_START + HEAP_SIZE);
static uint8_t *heap_ptr;

/* Inicialização   */

void memory_init(void)
{
    // Inicializa o ponteiro de alocação no início do heap
    heap_ptr = heap_base;
}

/* Alocador bump   */

void *kmalloc(uint64_t size)
{
    if (size == 0)
        return 0;

    // Alinha o tamanho para 8 bytes (melhor desempenho e compatibilidade)
    size = (size + 7) & ~7ULL;

    // Verifica se ainda há espaço no heap
    if (heap_ptr + size > heap_end)
        return 0;   // out of memory

    void *ptr = heap_ptr;

    // Avança o ponteiro (alocação linear, sem reutilização)
    heap_ptr += size;

    return ptr;
}

/* Free mínimo   */

void kfree(void *ptr)
{
    // Implementação simplificada: não há liberação real de memória
    (void)ptr;
}

/* Estatísticas   */

uint64_t memory_used(void)
{
    // Quantidade de memória já consumida do heap
    return (uint64_t)(heap_ptr - heap_base);
}

uint64_t memory_free(void)
{
    // Espaço restante disponível no heap
    return (uint64_t)(heap_end - heap_ptr);
}
