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

    /* Aloca a pilha da task */
    t->stack = (uint8_t *)kmalloc(stack_size);

    if (!t->stack)
        return;

    /* Calcula o topo da pilha e garante alinhamento de 16 bytes */
    uint64_t raw_sp = (uint64_t)(t->stack + stack_size);
    uint64_t *sp = (uint64_t *)(raw_sp & ~15ULL);

    /* Inicializa contexto da tarefa */
    for (int i = 0; i < 31; i++)
        t->regs[i] = 0;

    /* SP inicial da tarefa (x2) */
    t->regs[1] = (uint64_t)sp;

    /* Define ponto de entrada da execução */
    t->regs[0] = (uint64_t)task;

    /* Metadados da tarefa */
    t->entry = task;
    t->priority = priority;

    /* PC inicial usado pelo scheduler (sepc) */
    t->pc = (uint64_t)task;

    task_count++;
}
