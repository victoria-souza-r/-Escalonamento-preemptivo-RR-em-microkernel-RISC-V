#include "scheduler.h"
#include "task.h"
#include <stdint.h>
#include "uart.h"

/* Variável global controlando a tarefa atualmente em execução */
static int current = 0;

/* Implementação do Round-Robin utilizando o contador global */
static int round_robin(void)
{
    return (current + 1) % task_count;
}

static sched_algo_t current_algo = round_robin;

void scheduler_set_algorithm(sched_algo_t algo)
{
    // Permite trocar o algoritmo de escalonamento dinamicamente
    if (algo)
        current_algo = algo;
}

/* Escalonamento Preemptivo chamado pelo Trap Handler */
void schedule_from_trap(uint64_t *frame)
{
    // Salva o contexto da tarefa atual no TCB
    for (int i = 0; i < 31; i++)
        tasks[current].regs[i] = frame[i];

    // Captura o PC onde a tarefa foi interrompida
    uint64_t sepc;
    asm volatile("csrr %0, sepc" : "=r"(sepc));
    tasks[current].pc = sepc;

    // Seleciona a próxima tarefa conforme o algoritmo ativo
    int next = current_algo();
    current = next;

    // Restaura o contexto da nova tarefa para o frame de execução
    for (int i = 0; i < 31; i++)
        frame[i] = tasks[current].regs[i];

    // Ajusta o PC para retomar a execução da tarefa escolhida
    asm volatile("csrw sepc, %0" : : "r"(tasks[current].pc));
}

void scheduler_start(void)
{
    if (task_count == 0)
        return;

    /* Configuração do status do supervisor:
     * SPP = retorna em modo supervisor
     * SPIE = habilita interrupções após sret
     */
    uint64_t sstatus;
    asm volatile("csrr %0, sstatus" : "=r"(sstatus));
    sstatus |= (1 << 8) | (1 << 5);
    asm volatile("csrw sstatus, %0" : : "r"(sstatus));

    /* Inicia a primeira tarefa do sistema */
    asm volatile("csrw sepc, %0" : : "r"(tasks[0].pc));

    // Inicializa stack pointer da primeira task
    asm volatile("mv sp, %0" : : "r"(tasks[0].regs[REG_SP]));

    // Retorna para o fluxo da tarefa (transfere controle para o user/kernel task)
    asm volatile("sret");
}
