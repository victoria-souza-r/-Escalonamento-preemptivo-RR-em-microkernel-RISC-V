#pragma once
#include <stdint.h>

#define MAX_TASKS 8
#define DEFAULT_STACK_SIZE 2048

/* Índices dos registradores salvos no contexto */
typedef enum
{
    REG_RA = 0,   // x1
    REG_SP,       // x2
    REG_GP,       // x3
    REG_TP,       // x4

    REG_T0,       // x5
    REG_T1,       // x6
    REG_T2,       // x7

    REG_S0,       // x8
    REG_S1,       // x9

    REG_A0,       // x10
    REG_A1,       // x11
    REG_A2,       // x12
    REG_A3,       // x13
    REG_A4,       // x14
    REG_A5,       // x15
    REG_A6,       // x16
    REG_A7,       // x17

    REG_S2,       // x18
    REG_S3,       // x19
    REG_S4,       // x20
    REG_S5,       // x21
    REG_S6,       // x22
    REG_S7,       // x23
    REG_S8,       // x24
    REG_S9,       // x25
    REG_S10,      // x26
    REG_S11,      // x27

    REG_T3,       // x28
    REG_T4,       // x29
    REG_T5,       // x30
    REG_T6        // x31

} reg_index_t;

/* Task Control Block */
typedef struct
{
    uint64_t regs[31];      // x1–x31
    uint64_t pc;            // Program Counter salvo do CSR sepc

    void (*entry)(void);    // Função inicial da tarefa
    int priority;           // Prioridade (não utilizada pelo Round-Robin)
    uint8_t *stack;         // Base da pilha da tarefa

} TCB;

extern TCB tasks[MAX_TASKS];
extern int task_count;

void xTaskCreate(void (*task)(void),
                 uint32_t stack_size,
                 int priority);
