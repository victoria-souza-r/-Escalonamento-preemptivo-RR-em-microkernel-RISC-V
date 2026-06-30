#include "fs.h"
#include "block.h"
#include "string.h"

// Estruturas do sistema de arquivos 

static superblock_t superblock;
uint16_t fat[NUM_CLUSTERS];
static dir_entry_t directory[MAX_FILES];
static file_descriptor_t open_files[MAX_OPEN_FILES];


 // Funções auxiliares
 
// Procura um cluster livre na FAT 
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

// Procura um arquivo pelo nome 
static int find_file(const char *name)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (directory[i].used &&
            strcmp(directory[i].name, name) == 0)
        {
            return i;
        }
    }

    return -1;
}

// Procura uma entrada livre no diretório 
static int find_free_directory(void)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (!directory[i].used)
        {
            return i;
        }
    }

    return -1;
}

// API do sistema de arquivos

int fs_init(void)
{
    superblock_t sb_disk;

    // Tenta ler o superbloco do disco 
    if (block_read(0, &sb_disk) != 0)
    {
        return -1;
    }

    if (sb_disk.magic == FS_MAGIC)
    {
        // Disco já formatado 
        superblock = sb_disk;

        block_read(1, fat);
        block_read(2, directory);
    }
    else
    {
        // Formata o disco 

        superblock.magic = FS_MAGIC;
        superblock.total_blocks = NUM_BLOCKS;
        superblock.total_clusters = NUM_CLUSTERS;
        superblock.cluster_size = CLUSTER_SIZE;

        // Inicializa FAT 

        for (int i = 0; i < NUM_CLUSTERS; i++)
        {
            fat[i] = FAT_FREE;
        }

        // Inicializa diretório 

        memset(directory, 0, sizeof(directory));

        // Salva no disco 

        block_write(0, &superblock);
        block_write(1, fat);
        block_write(2, directory);
    }

    // Limpa tabela de arquivos abertos 

    memset(open_files, 0, sizeof(open_files));

    return 0;
}

int fs_create(const char *name)
{
    /* Ainda será implementada */

    return 0;
}

int fs_open(const char *name)
{
    /* Ainda será implementada */

    return 0;
}

int fs_close(int fd)
{
    /* Ainda será implementada */

    return 0;
}

int fs_read(int fd, void *buffer, uint32_t size)
{
    /* Ainda será implementada */

    return 0;
}

int fs_write(int fd, const void *buffer, uint32_t size)
{
    /* Ainda será implementada */

    return 0;
}

int fs_delete(const char *name)
{
    /* Ainda será implementada */

    return 0;
}
