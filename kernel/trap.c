#include "trap.h"
#include "timer.h"
#include "scheduler.h"
#include "uart.h"
#include <stdint.h>

void trap_handler(uint64_t *frame)
{
    uint64_t scause;
    asm volatile("csrr %0, scause" : "=r"(scause));

    // Verifica se é uma interrupção (bit 63 == 1) e se é de timer (código 5)
    if ((scause & (1ULL << 63)) && (scause & 0xFF) == 5)
    {
        timer_next();             // Agenda o próximo disparo
        schedule_from_trap(frame); // Escalonamento preemptivo
    }
    else
    {
        // Se cair aqui, o erro não é de Timer.
        uart_print("Unhandled trap! scause: 0x");
        // Opcional: imprime scause para debug
        while (1); 
    }
}
