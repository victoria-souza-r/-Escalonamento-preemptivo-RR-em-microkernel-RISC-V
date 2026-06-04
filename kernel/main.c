#include "task.h"
#include "scheduler.h"
#include "memory.h"
#include "timer.h" // Incluído para podermos inicializar o timer

extern void uart_print(const char*);
extern void uart_print_uint(uint64_t);
extern void trap_entry(void); // Ponto de entrada em Assembly do Trap Handler

/* Tasks   */

void task1()
{
    while (1)
    {
        uart_print("Task 1 running\n");

        uart_print("Memory used: ");
        uart_print_uint(memory_used());
        uart_print(" bytes\n");

        uart_print("Memory free: ");
        uart_print_uint(memory_free());
        uart_print(" bytes\n\n");

    }
}

void task2()
{
    while (1)
    {
        uart_print("Task 2 running\n");

        uart_print("Memory used: ");
        uart_print_uint(memory_used());
        uart_print(" bytes\n");

        uart_print("Memory free: ");
        uart_print_uint(memory_free());
        uart_print(" bytes\n\n");

    }
}
/* alternativa para somente visualizar
void task1()
{
    volatile uint64_t x = 0;

    while (1)
    {
        uart_print("Task 1\n");

        for (x = 0; x < 5000000; x++);
    }
}

void task2()
{
    volatile uint64_t x = 0;

    while (1)
    {
        uart_print("Task 2\n");

        for (x = 0; x < 5000000; x++);
    }
} */

/* Kernel   */

void kernel_main()
{
    memory_init(); 

    /* Configura o CSR stvec para apontar para o endereço do trap_entry */
    asm volatile("csrw stvec, %0" : : "r"(trap_entry));

    /* Inicializa o timer com o intervalo desejado */
    timer_init(100000);

    uart_print("\n=== Preemptive Kernel ===\n");

    xTaskCreate(task1, 2048, 1);
    xTaskCreate(task2, 2048, 1);

    scheduler_start();

    while (1);
}
