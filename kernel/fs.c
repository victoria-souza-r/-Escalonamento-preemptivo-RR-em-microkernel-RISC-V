#include "fs.h"
#include "block.h"
#include "string.h"

// Define que os dados começam após o Superbloco(0), FAT(1) e Diretório(2)
#define DATA_START_BLOCK 3

// Estruturas do sistema de arquivos 
static superblock_t superblock;
uint16_t fat[NUM_CLUSTERS];
static dir_entry_t directory[MAX_FILES];
static file_descriptor_t open_files[MAX_OPEN_FILES];

// Funções auxiliares de baixo nível (Clusters)
 
// Leitura de um cluster completo
static int cluster_read(uint16_t cluster, void *buffer)
{
    uint8_t *ptr = (uint8_t *)buffer;
    uint32_t first_block = DATA_START_BLOCK + (cluster * CLUSTER_SIZE);

    for (uint32_t i = 0; i < CLUSTER_SIZE; i++)
    {
        // Usa block_read para ler cada bloco do cluster
        if (block_read(first_block + i, ptr + (i * BLOCK_SIZE)) != 0)
        {
            return -1;
        }
    }
    return 0;
}

// Escrita de um cluster completo
static int cluster_write(uint16_t cluster, const void *buffer)
{
    const uint8_t *ptr = (const uint8_t *)buffer;
    uint32_t first_block = DATA_START_BLOCK + (cluster * CLUSTER_SIZE);

    for (uint32_t i = 0; i < CLUSTER_SIZE; i++)
    {
        // Usa block_write para escrever cada bloco do cluster
        if (block_write(first_block + i, ptr + (i * BLOCK_SIZE)) != 0)
        {
            return -1;
        }
    }
    return 0;
}

 // Funções auxiliares de lógica

static int cluster_alloc(void)
{
    for (int i = 0; i < NUM_CLUSTERS; i++)
    {
        if (fat[i] == FAT_FREE)
        {
            fat[i] = FAT_EOF;
            return i;
        }
    }
    return -1;
}

static int find_file(const char *name)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (directory[i].used && strcmp(directory[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

static int find_free_directory(void)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (!directory[i].used) return i;
    }
    return -1;
}

//  API do sistema de arquivos
int fs_init(void)
{
    superblock_t sb_disk;

    // Tenta ler o superbloco do disco (bloco 0)
    if (block_read(0, &sb_disk) != 0) return -1;

    if (sb_disk.magic == FS_MAGIC)
    {
        // Disco já formatado: carrega os dados
        superblock = sb_disk;
        block_read(1, fat);       // Bloco 1 contém a FAT
        block_read(2, directory); // Bloco 2 contém o Diretório
    }
    else
    {
        // Formata o disco pela primeira vez
        superblock.magic = FS_MAGIC;
        superblock.total_blocks = NUM_BLOCKS;
        superblock.total_clusters = NUM_CLUSTERS;
        superblock.cluster_size = CLUSTER_SIZE;

        // Correção: Inicializa a FAT via loop para garantir segurança de tipo
        for (int i = 0; i < NUM_CLUSTERS; i++)
        {
            fat[i] = FAT_FREE;
        }

        // memset é aceitável aqui pois directory é um array de struct 
        // e queremos zerar todos os bytes (como um reset total)
        memset(directory, 0, sizeof(directory));

        // Salva os metadados iniciais no disco
        block_write(0, &superblock);
        block_write(1, fat);
        block_write(2, directory);
    }

    // Limpa a tabela de descritores de arquivos em memória
    memset(open_files, 0, sizeof(open_files));
    
    return 0;
}

int fs_create(const char *name)
{
    if (name == NULL || strlen(name) == 0 || strlen(name) >= MAX_FILENAME) return -1;
    if (find_file(name) != -1) return -1;

    int dir_index = find_free_directory();
    if (dir_index == -1) return -1;

    int cluster = cluster_alloc();
    if (cluster == -1) return -1;

    strcpy(directory[dir_index].name, name);
    directory[dir_index].size = 0;
    directory[dir_index].first_cluster = cluster;
    directory[dir_index].used = 1;

    // Atualiza metadados via block_write
    block_write(1, fat);
    block_write(2, directory);

    return 0;
}

int fs_open(const char *name)
{
    int dir_index = find_file(name);
    if (dir_index == -1) return -1;

    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (!open_files[i].used)
        {
            open_files[i].used = 1;
            open_files[i].position = 0;
            open_files[i].dir_index = dir_index;
            return i;
        }
    }
    return -1;
}

int fs_close(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].used) return -1;
    open_files[fd].used = 0;
    return 0;
}

int fs_write(int fd, const void *buffer, uint32_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].used) return -1;

    dir_entry_t *file = &directory[open_files[fd].dir_index];
    uint32_t max_size = BLOCK_SIZE * CLUSTER_SIZE;
    if (size > max_size) size = max_size;

    // A função cluster_write já calcula o DATA_START_BLOCK, 
    // então passamos apenas o índice do cluster.
    if (cluster_write(file->first_cluster, buffer) != 0) return -1;

    file->size = size;
    open_files[fd].position = size;

    block_write(2, directory); // Atualiza diretório no disco
    return size;
}

int fs_read(int fd, void *buffer, uint32_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].used) return -1;

    dir_entry_t *file = &directory[open_files[fd].dir_index];
    if (size > file->size) size = file->size;

    // A função cluster_read já calcula o DATA_START_BLOCK
    if (cluster_read(file->first_cluster, buffer) != 0) return -1;

    open_files[fd].position = size;
    return size;
}

int fs_delete(const char *name)
{
    int dir_index = find_file(name);
    if (dir_index == -1) return -1;

    uint16_t current_cluster = directory[dir_index].first_cluster;

    while (current_cluster != FAT_EOF)
    {
        uint16_t next = fat[current_cluster];
        fat[current_cluster] = FAT_FREE;
        if (next == FAT_EOF) break;
        current_cluster = next;
    }

    memset(&directory[dir_index], 0, sizeof(dir_entry_t));

    // Atualiza metadados via block_write
    block_write(1, fat);
    block_write(2, directory);

    return 0;
}
