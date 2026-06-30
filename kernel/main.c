#include "task.h"
#include "scheduler.h"
#include "memory.h"
#include "timer.h"
#include "fs.h"
#include "string.h"

extern void uart_print(const char *);
extern void uart_print_uint(uint64_t);
extern void trap_entry(void);

// Declaração do array fat que está em fs.c
extern uint16_t fat[NUM_CLUSTERS];

// Função de debug para imprimir o estado da FAT e validar o encadeamento
void print_fat_debug()
{
    uart_print("\n--- Estado da FAT (Encadeamento de Clusters) ---\n");
    for (int i = 0; i < NUM_CLUSTERS; i++)
    {
        if (fat[i] != FAT_FREE)
        {
            uart_print("Cluster ");
            uart_print_uint(i);
            uart_print(" -> ");
            if (fat[i] == FAT_EOF)
                uart_print("EOF");
            else
                uart_print_uint(fat[i]);
            uart_print("\n");
        }
    }
    uart_print("------------------------------------------------\n\n");
}

// Tasks de exemplo
void task1() { while (1) { /* ... */ } }
void task2() { while (1) { /* ... */ } }

void kernel_main()
{
    uart_print("Entrou no kernel!\n");
    memory_init();

    // Configura o trap handler
    asm volatile("csrw stvec, %0" : : "r"(trap_entry));

    // Inicializa o timer
    timer_init(100000);

    uart_print("\n=== Preemptive Kernel com SimpleFAT ===\n");

    // 1. Inicialização
    fs_init();

    // 2. Criação
    uart_print("Criando 'notas.txt'...\n");
    if (fs_create("notas.txt") == 0)
    {
        // 3. Abertura
        int fd = fs_open("notas.txt");
        if (fd >= 0)
        {
            char msg[] = "Bem-vindo ao SimpleFAT";
            
            // 4. Escrita
            uart_print("Escrevendo: ");
            uart_print(msg);
            uart_print("\n");
            fs_write(fd, msg, sizeof(msg));

            // Debug: Ver encadeamento de clusters
            print_fat_debug();

            // 5. Leitura
            char buffer[64];
            memset(buffer, 0, sizeof(buffer));
            fs_read(fd, buffer, sizeof(msg));

            uart_print("Conteudo lido do disco: ");
            uart_print(buffer);
            uart_print("\n");

            // 6. Fechamento
            fs_close(fd);
            uart_print("Arquivo fechado.\n");
        }

        // 7. Remoção
        uart_print("Removendo 'notas.txt'...\n");
        if (fs_delete("notas.txt") == 0)
        {
            uart_print("Arquivo removido com sucesso.\n");
            print_fat_debug(); // Deve mostrar a FAT vazia/limpa
        }
    }
    else
    {
        uart_print("Erro ao criar arquivo.\n");
    }

    uart_print("Teste do sistema de arquivos concluido.\n\n");

    // Inicia tarefas
    xTaskCreate(task1, 2048, 1);
    xTaskCreate(task2, 2048, 1);
    scheduler_start();

    while (1);
}
