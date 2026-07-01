#ifndef FS_H
#define FS_H

#include <stdint.h>
#include "block.h"

/* Configuração do sistema de arquivos */

#define CLUSTER_SIZE      1
#define NUM_CLUSTERS      (NUM_BLOCKS / CLUSTER_SIZE)

#define MAX_FILES         64
#define MAX_FILENAME      32
#define MAX_OPEN_FILES    16

/* Assinatura do SimpleFAT */
#define FS_MAGIC          0x53464154

/* Constantes da FAT */
#define FAT_FREE          0x0000
#define FAT_EOF           0xFFFF

/* Superbloco */
typedef struct
{
    uint32_t magic;
    uint32_t total_blocks;
    uint32_t total_clusters;
    uint32_t cluster_size;
} superblock_t;

/* Entrada do diretório raiz */
typedef struct
{
    char name[MAX_FILENAME];
    uint32_t size;
    uint16_t first_cluster;
    uint8_t used;
} dir_entry_t;

/* Descritor de arquivo aberto */
typedef struct
{
    uint8_t used;
    uint32_t position;
    int16_t dir_index;
} file_descriptor_t;

/* API do sistema de arquivos */
int fs_init(void);

int fs_create(const char *name);
int fs_open(const char *name);
int fs_close(int fd);

int fs_read(int fd, void *buffer, uint32_t size);
int fs_write(int fd, const void *buffer, uint32_t size);

int fs_delete(const char *name);

/* Funções auxiliares */
int cluster_alloc(void);
void cluster_free_chain(uint16_t cluster);

#endif
