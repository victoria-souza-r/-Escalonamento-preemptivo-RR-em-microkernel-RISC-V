#include "trap.h"
#include "timer.h"
#include "scheduler.h"
#include "uart.h"
#include <stdint.h>

void trap_handler(uint64_t *frame)
{
    uint64_t scause;
    asm volatile("csrr %0, scause" : "=r"(scause));

    // Identifica o tipo de trap (interrupção ou exceção)
    // Timer interrupt: bit 63 = 1 (interrupt) e código 5
    if ((scause & (1ULL << 63)) && (scause & 0xFF) == 5)
    {
        // Atualiza o próximo evento do timer (mantém interrupções periódicas)
        timer_next();

        // Executa o escalonador preemptivo a partir da interrupção
        schedule_from_trap(frame);
    }
    else
    {
        // Trap não tratada pelo kernel
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

        // Para o sistema em caso de erro crítico
        while (1);
    }
}
