#include "task.h"
#include "memory.h"
#include <stdint.h>

TCB tasks[MAX_TASKS];
int task_count = 0;

void xTaskCreate(void (*task)(void),
                 uint32_t stack_size,
                 int priority)
{
    if (task_count >= MAX_TASKS)
        return;

    if (stack_size == 0)
        stack_size = DEFAULT_STACK_SIZE;

    TCB *t = &tasks[task_count];

    /* Aloca a pilha */
    t->stack = (uint8_t *)kmalloc(stack_size);

    if (!t->stack)
        return;

    /* Força o topo da pilha (sp) a estar alinhado em 16 bytes */
    uint64_t raw_sp = (uint64_t)(t->stack + stack_size);
    uint64_t *sp = (uint64_t *)(raw_sp & ~15ULL);

    /* Limpa todos os registradores */
    for (int i = 0; i < 31; i++)
        t->regs[i] = 0;

    /* SP inicial da tarefa mapeado em x2 (regs[1]) */
    t->regs[1] = (uint64_t)sp;

    /* Registrador ra (x1) recebe o endereço inicial por segurança */
    t->regs[0] = (uint64_t)task;

    /* Guarda informações da tarefa */
    t->entry = task;
    t->priority = priority;

    /* O scheduler preemptivo usa pc para alimentar o CSR sepc */
    t->pc = (uint64_t)task;

    task_count++;
}
