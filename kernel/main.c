#include "task.h"
#include "scheduler.h"
#include "memory.h"
#include "timer.h"
#include "fs.h"
#include "block.h"

extern void uart_print(const char *);
extern void uart_print_uint(uint64_t);
extern void trap_entry(void);

// Lê um bloco do disco e exibe seu conteúdo pela UART
void mostrar_bloco(uint32_t bloco)
{
    char buffer[BLOCK_SIZE + 1];

    // Carrega o conteúdo do bloco para um buffer em memória
    block_read(bloco, buffer);

    // Garante que o buffer seja tratado como uma string
    buffer[BLOCK_SIZE] = '\0';

    uart_print("\nBloco ");
    uart_print_uint(bloco);
    uart_print(":\n");

    // Exibe o conteúdo do bloco
    uart_print(buffer);
    uart_print("\n");
}

// Task de teste do SimpleFAT
void fs_task()
{
    char buffer[64];
    int fd;

    uart_print("\n=== Inicializando SimpleFAT ===\n");

    // Inicializa as estruturas do sistema de arquivos em memória
    if (fs_init() == 0)
        uart_print("Sistema de arquivos inicializado.\n");
    else
    {
        uart_print("Erro ao inicializar o sistema de arquivos.\n");

        // Interrompe a execução caso a inicialização falhe
        while (1);
    }

        /* Cenário 1 */
    uart_print("\n[CENARIO 1] Criando notas.txt...\n");

    // Cria um novo arquivo no diretório raiz
    if (fs_create("notas.txt") == 0)
        uart_print("Arquivo criado com sucesso.\n");
    else
        uart_print("Erro ao criar arquivo.\n");

    /* Cenário 2 */
    uart_print("\n[CENARIO 2] Abrindo e escrevendo...\n");

    // Obtém um descritor para acessar o arquivo
    fd = fs_open("notas.txt");

    if (fd < 0)
    {
        uart_print("Erro ao abrir arquivo.\n");

        // Não é possível continuar sem abrir o arquivo
        while (1);
    }

    /* Para testar a gravação em vários blocos (clusters),
     
      char texto[1500];
     
      for (int i = 0; i < sizeof(texto) - 1; i++)
      {
          texto[i] = 'A' + (i % 26);
      }
     
      texto[1499] = '\0';
     
      fs_write(fd, texto, 1500); */
     

        // Escreve os dados no arquivo aberto
    fs_write(fd,
             "Bem-vindo ao SimpleFAT",
             22);

    uart_print("Dados escritos.\n");

    uart_print("\n=== Disco Virtual ===\n");

    // Exibe os primeiros blocos para verificar o estado do disco
    for (int i = 0; i < 5; i++)
    {
        mostrar_bloco(i);
    }

    /* Cenário 3 */
    uart_print("\n[CENARIO 3] Lendo arquivo...\n");

    // Lê o conteúdo armazenado no arquivo
    fs_read(fd, buffer, 22);
    buffer[22] = '\0';

    uart_print("Conteudo: ");
    uart_print(buffer);
    uart_print("\n");

    // Libera o descritor de arquivo
    fs_close(fd);

    /* Cenário 4 */
    uart_print("\n[CENARIO 4] Removendo arquivo...\n");

    // Remove o arquivo e libera seus clusters
    if (fs_delete("notas.txt") == 0)
        uart_print("Arquivo removido com sucesso.\n");
    else
        uart_print("Erro ao remover arquivo.\n");

    /* Cenário 5 */
    uart_print("\n[CENARIO 5] Reutilizacao de espaco...\n");

    // Verifica se o espaço liberado pode ser reutilizado
    if (fs_create("novo.txt") == 0)
        uart_print("Espaco reutilizado com sucesso.\n");
    else
        uart_print("Falha na reutilizacao de espaco.\n");

    /* Cenário extra */
    uart_print("\n[CENARIO EXTRA] Criando varios arquivos...\n");

    // Testa a criação de múltiplos arquivos consecutivos
    fs_create("a.txt");
    fs_create("b.txt");
    fs_create("c.txt");

    uart_print("Arquivos a.txt, b.txt e c.txt criados.\n");

    uart_print("\n=== TESTES FINALIZADOS ===\n");

    // Mantém a task em execução após o término dos testes
    while (1)
        ;
}

// Kernel
void kernel_main()
{
    // Inicializa o gerenciador de memória
    memory_init();

    /* Configura o vetor de interrupções */
    asm volatile("csrw stvec, %0"
                 :
                 : "r"(trap_entry));

    // Configura interrupções periódicas do timer
    timer_init(100000);

    uart_print("\n=== Preemptive Kernel + SimpleFAT ===\n");

    // Cria a task responsável pelos testes do sistema de arquivos
    xTaskCreate(fs_task, 2048, 1);

    // Inicia o escalonador e transfere o controle para as tasks
    scheduler_start();

    // O kernel não deve retornar após iniciar o escalonador
    while (1)
        ;
}
