#include "task.h"
#include "scheduler.h"
#include "memory.h"
#include "timer.h"
#include "fs.h"
#include "block.h"

extern void uart_print(const char *);
extern void uart_print_uint(uint64_t);
extern void trap_entry(void);

// Mostra o conteúdo de um bloco
void mostrar_bloco(uint32_t bloco)
{
    char buffer[BLOCK_SIZE + 1];

    block_read(bloco, buffer);

    buffer[BLOCK_SIZE] = '\0';

    uart_print("\nBloco ");
    uart_print_uint(bloco);
    uart_print(":\n");

    uart_print(buffer);
    uart_print("\n");
}

// Task de teste do SimpleFAT
void fs_task()
{
    char buffer[64];
    int fd;

    uart_print("\n=== Inicializando SimpleFAT ===\n");

    if (fs_init() == 0)
        uart_print("Sistema de arquivos inicializado.\n");
    else
    {
        uart_print("Erro ao inicializar o sistema de arquivos.\n");
        while (1);
    }

    /* Cenário 1 */
    uart_print("\n[CENARIO 1] Criando notas.txt...\n");

    if (fs_create("notas.txt") == 0)
        uart_print("Arquivo criado com sucesso.\n");
    else
        uart_print("Erro ao criar arquivo.\n");

    /* Cenário 2 */
    uart_print("\n[CENARIO 2] Abrindo e escrevendo...\n");

    fd = fs_open("notas.txt");

    if (fd < 0)
    {
        uart_print("Erro ao abrir arquivo.\n");
        while (1);
    }

    fs_write(fd,
             "Bem-vindo ao SimpleFAT",
             22);

    uart_print("Dados escritos.\n");

    uart_print("\n=== Disco Virtual ===\n");

    for (int i = 0; i < 5; i++)
    {
        mostrar_bloco(i);
    }

    /* Cenário 3 */
    uart_print("\n[CENARIO 3] Lendo arquivo...\n");

    fs_read(fd, buffer, 22);
    buffer[22] = '\0';

    uart_print("Conteudo: ");
    uart_print(buffer);
    uart_print("\n");

    fs_close(fd);

    /* Cenário 4 */
    uart_print("\n[CENARIO 4] Removendo arquivo...\n");

    if (fs_delete("notas.txt") == 0)
        uart_print("Arquivo removido com sucesso.\n");
    else
        uart_print("Erro ao remover arquivo.\n");

    /* Cenário 5 */
    uart_print("\n[CENARIO 5] Reutilizacao de espaco...\n");

    if (fs_create("novo.txt") == 0)
        uart_print("Espaco reutilizado com sucesso.\n");
    else
        uart_print("Falha na reutilizacao de espaco.\n");

    /* Cenário extra */
    uart_print("\n[CENARIO EXTRA] Criando varios arquivos...\n");

    fs_create("a.txt");
    fs_create("b.txt");
    fs_create("c.txt");

    uart_print("Arquivos a.txt, b.txt e c.txt criados.\n");

    uart_print("\n=== TESTES FINALIZADOS ===\n");

    while (1)
        ;
}

// Kernel
void kernel_main()
{
    memory_init();

    /* Configura o vetor de interrupções */
    asm volatile("csrw stvec, %0"
                 :
                 : "r"(trap_entry));

    /* Inicializa o timer */
    timer_init(100000);

    uart_print("\n=== Preemptive Kernel + SimpleFAT ===\n");

    /* Cria a task de teste */
    xTaskCreate(fs_task, 2048, 1);

    /* Inicia o escalonador */
    scheduler_start();

    while (1)
        ;
}
