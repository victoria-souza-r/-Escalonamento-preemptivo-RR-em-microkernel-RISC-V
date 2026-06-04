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

    /* Topo da pilha */
    uint64_t *sp = (uint64_t *)(t->stack + stack_size);

    /* Limpa todos os registradores */
    for (int i = 0; i < 31; i++)
        t->regs[i] = 0;

    /*
     * Layout:
     * regs[0]  = x1 (ra)
     * regs[1]  = x2 (sp)
     * regs[2]  = x3
     * ...
     * regs[30] = x31
     */

    /* SP inicial da tarefa */
    t->regs[1] = (uint64_t)sp;

    /*
     * Quando a tarefa for restaurada pela primeira vez,
     * o retorno deverá cair na função da task.
     */
    t->regs[0] = (uint64_t)task;

    /* Guarda informações da tarefa */
    t->entry = task;
    t->priority = priority;

    /*
     * O scheduler preemptivo usa pc/sepc para restaurar
     * a execução da tarefa após uma interrupção.
     */
    t->pc = (uint64_t)task;

    task_count++;
}
