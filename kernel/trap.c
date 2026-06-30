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
    uart_print("\nUnhandled trap!\n");

    uart_print("scause = ");
    uart_print_uint(scause);
    uart_print("\n");

    uint64_t sepc;
    asm volatile("csrr %0, sepc" : "=r"(sepc));

    uart_print("sepc = ");
    uart_print_uint(sepc);
    uart_print("\n");

    uint64_t stval;
    asm volatile("csrr %0, stval" : "=r"(stval));

    uart_print("stval = ");
    uart_print_uint(stval);
    uart_print("\n");

    while (1);
}
 } 
