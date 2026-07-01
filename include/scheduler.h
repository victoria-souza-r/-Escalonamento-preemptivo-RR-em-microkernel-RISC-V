#pragma once

#include <stdint.h>

// Tipo de função utilizada para selecionar a próxima task
typedef int (*sched_algo_t)(void);

void scheduler_set_algorithm(sched_algo_t algo);
void scheduler_start(void);
void yield(void);

// Chamado pelo tratador de interrupções para realizar a troca de contexto
void schedule_from_trap(uint64_t *frame);
