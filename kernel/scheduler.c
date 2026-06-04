#include "scheduler.h"
#include "task.h"
#include <stdint.h>

extern void context_switch(void*, void*);

static int current = 0;

/* Round-Robin */
static int round_robin(void)
{
    return (current + 1) % task_count;
}

/* Algoritmo atual   */
static sched_algo_t current_algo = round_robin;

void scheduler_set_algorithm(sched_algo_t algo)
{
    if (algo)
        current_algo = algo;
}

/* Yield (Escalonamento Cooperativo)   */
void yield(void)
{
    int prev = current;
    int next = current_algo();

    current = next;

    context_switch(tasks[prev].regs,
                   tasks[next].regs);
}

/* Escalonamento Preemptivo a partir do Trap   */
void schedule_from_trap(uint64_t *frame)
{
    /* Salva contexto atual */
    for (int i = 0; i < 31; i++)
        tasks[current].regs[i] = frame[i];

    uint64_t sepc;
    asm volatile("csrr %0, sepc" : "=r"(sepc));

    tasks[current].pc = sepc;

    /* Escolhe próxima task */
    int next = current_algo();

    current = next;

    /* Carrega contexto da próxima task */
    for (int i = 0; i < 31; i++)
        frame[i] = tasks[current].regs[i];

    asm volatile("csrw sepc, %0"
                 :
                 : "r"(tasks[current].pc));
}
 
void scheduler_start(void)
{
    if (task_count == 0)
        return;

    /* * Em vez de chamar a função diretamente, configuramos o sepc para o 
     * início da tarefa e o sp para o topo da pilha da tarefa.
     * O 'sret' então fará o salto, iniciando a execução da Task 1.
     */
    asm volatile("csrw sepc, %0" : : "r"(tasks[0].pc));
    asm volatile("mv sp, %0" : : "r"(tasks[0].regs[1])); // regs[1] guarda o SP inicial
    asm volatile("sret");
}
