#include "task.h"
#include "scheduler.h"
#include "memory.h"
#include "timer.h"
#include "fs.h"
#include "string.h"

extern void uart_print(const char *);
extern void uart_print_uint(uint64_t);
extern void trap_entry(void);

// Tasks
 
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
}

// Kernel


void kernel_main()
{
    uart_print("Entrou no kernel!\n");

    memory_init();

    // Configura o trap handler 
    asm volatile("csrw stvec, %0" : : "r"(trap_entry));

    // Inicializa o timer 
    timer_init(100000);

    uart_print("\n=== Preemptive Kernel ===\n");

    // TESTE DO SISTEMA DE ARQUIVOS

    uart_print("Inicializando FS...\n");
    fs_init();

    uart_print("Criando arquivo...\n");
    if (fs_create("notas.txt") == 0)
    {
        uart_print("Arquivo criado com sucesso.\n");
    }
    else
    {
        uart_print("Erro ao criar arquivo.\n");
    }

    int fd = fs_open("notas.txt");

    if (fd >= 0)
    {
        uart_print("Arquivo aberto.\n");

        char msg[] = "Bem-vindo ao SimpleFAT";
        char buffer[64];

        memset(buffer, 0, sizeof(buffer));

        uart_print("Escrevendo no arquivo...\n");
        fs_write(fd, msg, sizeof(msg));

        uart_print("Lendo do arquivo...\n");
        fs_read(fd, buffer, sizeof(msg));

        uart_print("Conteudo lido: ");
        uart_print(buffer);
        uart_print("\n");

        fs_close(fd);
        uart_print("Arquivo fechado.\n");
    }
    else
    {
        uart_print("Erro ao abrir arquivo.\n");
    }

    if (fs_delete("notas.txt") == 0)
    {
        uart_print("Arquivo removido.\n");
    }
    else
    {
        uart_print("Erro ao remover arquivo.\n");
    }

    uart_print("Teste do sistema de arquivos concluido.\n\n");

    // Inicia as tarefas do kernel
    xTaskCreate(task1, 2048, 1);
    xTaskCreate(task2, 2048, 1);

    scheduler_start();

    while (1);
}
