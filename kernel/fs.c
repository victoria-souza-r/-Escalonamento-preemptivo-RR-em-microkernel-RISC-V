#include "fs.h"
#include "block.h"
#include "string.h"
#include <stddef.h>

// Define que os dados começam após o Superbloco(0), FAT(1) e Diretório(2)
#define DATA_START_BLOCK 3

// Estruturas do sistema de arquivos 
static superblock_t superblock;
uint16_t fat[NUM_CLUSTERS];
static dir_entry_t directory[MAX_FILES];
static file_descriptor_t open_files[MAX_OPEN_FILES];

// Funções auxiliares de baixo nível (Clusters)

static int cluster_read(uint16_t cluster, void *buffer)
{
    uint8_t *ptr = (uint8_t *)buffer;
    uint32_t first_block = DATA_START_BLOCK + (cluster * CLUSTER_SIZE);

    for (uint32_t i = 0; i < CLUSTER_SIZE; i++)
    {
        if (block_read(first_block + i, ptr + (i * BLOCK_SIZE)) != 0)
            return -1;
    }
    return 0;
}

static int cluster_write(uint16_t cluster, const void *buffer)
{
    const uint8_t *ptr = (const uint8_t *)buffer;
    uint32_t first_block = DATA_START_BLOCK + (cluster * CLUSTER_SIZE);

    for (uint32_t i = 0; i < CLUSTER_SIZE; i++)
    {
        if (block_write(first_block + i, ptr + (i * BLOCK_SIZE)) != 0)
            return -1;
    }
    return 0;
}

static int cluster_alloc(void)
{
    for (int i = 0; i < NUM_CLUSTERS; i++)
    {
        if (fat[i] == FAT_FREE)
        {
            fat[i] = FAT_EOF; // Temporário
            return i;
        }
    }
    return -1;
}

// Monta a cadeia na FAT
static int cluster_chain_alloc(uint32_t count)
{
    if (count == 0) return FAT_EOF; // Prevenção para arquivos de tamanho 0

    int first = -1;
    int previous = -1;

    for (uint32_t i = 0; i < count; i++)
    {
        int current = cluster_alloc();

        if (current == -1) 
        {
            // Tratamento de memory leak: libera a cadeia parcialmente alocada
            if (first != -1) 
            {
                uint16_t c = first;
                while (c != FAT_EOF)
                {
                    uint16_t next = fat[c];
                    fat[c] = FAT_FREE;
                    c = next;
                }
            }
            return -1;
        }

        if (first == -1) first = current;
        if (previous != -1) fat[previous] = current;

        previous = current;
    }

    if (previous != -1) fat[previous] = FAT_EOF;

    return first;
}

static int find_file(const char *name)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (directory[i].used && strcmp(directory[i].name, name) == 0)
            return i;
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

// API do sistema de arquivos

int fs_init(void)
{
    superblock_t sb_disk;
    if (block_read(0, &sb_disk) != 0) return -1;

    if (sb_disk.magic == FS_MAGIC)
    {
        superblock = sb_disk;
        block_read(1, fat);
        block_read(2, directory);
    }
    else
    {
        superblock.magic = FS_MAGIC;
        superblock.total_blocks = NUM_BLOCKS;
        superblock.total_clusters = NUM_CLUSTERS;
        superblock.cluster_size = CLUSTER_SIZE;

        for (int i = 0; i < NUM_CLUSTERS; i++) fat[i] = FAT_FREE;
        memset(directory, 0, sizeof(directory));

        block_write(0, &superblock);
        block_write(1, fat);
        block_write(2, directory);
    }
    memset(open_files, 0, sizeof(open_files));
    return 0;
}

int fs_create(const char *name)
{
    if (name == NULL || strlen(name) == 0 || strlen(name) >= MAX_FILENAME) return -1;
    if (find_file(name) != -1) return -1;

    int dir_index = find_free_directory();
    if (dir_index == -1) return -1;

    strcpy(directory[dir_index].name, name);
    directory[dir_index].size = 0;
    directory[dir_index].first_cluster = FAT_EOF; // Arquivo nasce sem clusters alocados
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
    
    // Limpeza completa do descritor
    open_files[fd].used = 0;
    open_files[fd].position = 0;
    open_files[fd].dir_index = -1;
    
    return 0;
}

int fs_write(int fd, const void *buffer, uint32_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].used) return -1;

    dir_entry_t *file = &directory[open_files[fd].dir_index];
    const uint8_t *ptr = (const uint8_t *)buffer;

    //  Liberar a cadeia de clusters anterior, se houver
    uint16_t current_free = file->first_cluster;
    while (current_free != FAT_EOF && current_free != FAT_FREE)
    {
        uint16_t next = fat[current_free];
        fat[current_free] = FAT_FREE;
        current_free = next;
    }

    //  Calcular quantos clusters são necessários
    uint32_t cluster_bytes = BLOCK_SIZE * CLUSTER_SIZE;
    uint32_t clusters_needed = (size + cluster_bytes - 1) / cluster_bytes;

    //  Montar a cadeia (Aloca os novos clusters e liga na FAT)
    int first = cluster_chain_alloc(clusters_needed);
    if (first == -1 && size > 0) return -1;

    // Atualiza metadados do arquivo usando cast apropriado
    file->first_cluster = (uint16_t)first;
    file->size = size;

    // Percorrer a FAT e escrever os dados com buffer temporário estático
    uint16_t current = file->first_cluster;
    uint32_t offset = 0;

    // Utiliza buffer estático para não estourar a stack do kernel
    static uint8_t temp[BLOCK_SIZE * CLUSTER_SIZE];

    while (current != FAT_EOF)
    {
        uint32_t remaining = size - offset;
        if (remaining > cluster_bytes)
        {
            remaining = cluster_bytes;
        }

        memset(temp, 0, sizeof(temp));
        memcpy(temp, ptr + offset, remaining);

        // Adicionada a validação sugerida
        if (cluster_write(current, temp) != 0)
        {
            return -1;
        }

        offset += remaining;
        current = fat[current]; // Pula para o próximo na cadeia
    }

    // Salva FAT e Diretório atualizados
    block_write(1, fat);
    block_write(2, directory);

    open_files[fd].position = size;
    return size;
}

int fs_read(int fd, void *buffer, uint32_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].used) return -1;

    dir_entry_t *file = &directory[open_files[fd].dir_index];
    uint8_t *ptr = (uint8_t *)buffer;

    if (size > file->size) size = file->size;

    // Percorre a cadeia para ler os dados com buffer temporário estático
    uint16_t current = file->first_cluster;
    uint32_t offset = 0;
    uint32_t bytes_read = 0;
    uint32_t cluster_bytes = BLOCK_SIZE * CLUSTER_SIZE;

    // Utiliza buffer estático para não estourar a stack do kernel
    static uint8_t temp[BLOCK_SIZE * CLUSTER_SIZE];

    while (current != FAT_EOF && bytes_read < size)
    {
        // Adicionada a validação sugerida
        if (cluster_read(current, temp) != 0)
        {
            return -1;
        }

        uint32_t remaining = size - bytes_read;
        if (remaining > cluster_bytes)
        {
            remaining = cluster_bytes;
        }

        memcpy(ptr + offset, temp, remaining);

        offset += remaining;
        bytes_read += remaining;
        current = fat[current];
    }

    open_files[fd].position = bytes_read;
    return bytes_read; // Retorna a quantidade exata de bytes lidos
}

int fs_delete(const char *name)
{
    int dir_index = find_file(name);
    if (dir_index == -1) return -1;

    uint16_t current_cluster = directory[dir_index].first_cluster;

    while (current_cluster != FAT_EOF && current_cluster != FAT_FREE)
    {
        uint16_t next = fat[current_cluster];
        fat[current_cluster] = FAT_FREE;
        current_cluster = next;
    }

    memset(&directory[dir_index], 0, sizeof(dir_entry_t));

    // Fecha os descritores quando o arquivo é apagado
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (open_files[i].used && open_files[i].dir_index == dir_index)
        {
            open_files[i].used = 0;
            open_files[i].position = 0;
            open_files[i].dir_index = -1;
        }
    }

    // Atualiza metadados via block_write
    block_write(1, fat);
    block_write(2, directory);

    return 0;
}
