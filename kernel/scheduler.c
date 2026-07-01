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
    if (algo)
        current_algo = algo;
}

/* Escalonamento Preemptivo chamado pelo Trap Handler */
void schedule_from_trap(uint64_t *frame)
{
    /* Salva o contexto da tarefa interrompida no TCB */
    for (int i = 0; i < 31; i++)
        tasks[current].regs[i] = frame[i];

    /* Captura o PC onde a tarefa parou e salva no TCB */
    uint64_t sepc;
    asm volatile("csrr %0, sepc" : "=r"(sepc));
    tasks[current].pc = sepc;

    /* Seleciona a próxima tarefa (Lógica Round-Robin) */
    int next = current_algo();
    current = next;

    /* Carrega o contexto da nova tarefa no frame (que será restaurado pelo Assembly) */
    for (int i = 0; i < 31; i++)
        frame[i] = tasks[current].regs[i];

    /* Prepara o sepc para o retorno */
    asm volatile("csrw sepc, %0" : : "r"(tasks[current].pc));
}

void scheduler_start(void)
{
    if (task_count == 0)
        return;

    /* * Preparação do Status do Supervisor:
     * - SPP (bit 8): Seta para 1 (Supervisor Previous Privilege) para retornar ao S-mode após sret
     * - SPIE (bit 5): Habilita interrupções após o sret
     */
    uint64_t sstatus;
    asm volatile("csrr %0, sstatus" : "=r"(sstatus));
    sstatus |= (1 << 8) | (1 << 5); 
    asm volatile("csrw sstatus, %0" : : "r"(sstatus));

    /* * Início da primeira tarefa:
     * Carrega o PC da primeira task no CSR sepc e o SP original no registrador sp
     */
    asm volatile("csrw sepc, %0" : : "r"(tasks[0].pc));
    asm volatile("mv sp, %0" : : "r"(tasks[0].regs[REG_SP])); 
    asm volatile("sret");
}
