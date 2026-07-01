#include "fs.h"
#include "string.h"
#include <stddef.h>

/* Estruturas internas */

// Estruturas mantidas em memória pelo sistema de arquivos
static superblock_t superblock;
static uint16_t fat[NUM_CLUSTERS];
static dir_entry_t root_dir[MAX_FILES];
static file_descriptor_t open_files[MAX_OPEN_FILES];


/* Funções auxiliares */

// Procura um arquivo pelo nome no diretório raiz
static int find_file(const char *name)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (root_dir[i].used &&
            strcmp(root_dir[i].name, name) == 0)
        {
            return i;
        }
    }

    // Arquivo não encontrado
    return -1;
}

int cluster_alloc(void)
{
    /* Cluster 0 é reservado */

    // Procura o primeiro cluster livre na FAT
    for (uint16_t i = 1; i < NUM_CLUSTERS; i++)
    {
        if (fat[i] == FAT_FREE)
        {
            // Marca o cluster como fim da cadeia
            fat[i] = FAT_EOF;
            return i;
        }
    }

    // Não há clusters livres
    return -1;
}

void cluster_free_chain(uint16_t cluster)
{
    // Libera todos os clusters pertencentes ao arquivo
    while (cluster != FAT_EOF &&
           cluster < NUM_CLUSTERS)
    {
        uint16_t next = fat[cluster];

        fat[cluster] = FAT_FREE;

        if (next == FAT_EOF)
            break;

        cluster = next;
    }
}

/* Sistema de arquivos */
int fs_init(void)
{
    // Inicializa as informações do superbloco
    superblock.magic = FS_MAGIC;
    superblock.total_blocks = NUM_BLOCKS;
    superblock.total_clusters = NUM_CLUSTERS;
    superblock.cluster_size = CLUSTER_SIZE;

    // Limpa todas as estruturas do sistema de arquivos
    memset(fat, 0, sizeof(fat));
    memset(root_dir, 0, sizeof(root_dir));
    memset(open_files, 0, sizeof(open_files));

    return 0;
}

int fs_create(const char *name)
{
    // Verifica se o nome é válido
    if (name == NULL)
        return -1;

    // Não permite arquivos com nomes repetidos
    if (find_file(name) >= 0)
        return -1;

    int entry = -1;

    // Procura uma posição livre no diretório raiz
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (!root_dir[i].used)
        {
            entry = i;
            break;
        }
    }

    if (entry < 0)
        return -1;

    // Reserva o primeiro cluster do arquivo
    int cluster = cluster_alloc();

    if (cluster < 0)
        return -1;

    // Inicializa a entrada do novo arquivo
    root_dir[entry].used = 1;
    root_dir[entry].size = 0;
    root_dir[entry].first_cluster = cluster;

    strncpy(root_dir[entry].name,
            name,
            MAX_FILENAME - 1);

    root_dir[entry].name[MAX_FILENAME - 1] = '\0';

    return 0;
}

int fs_open(const char *name)
{
    // Localiza o arquivo no diretório
    int file = find_file(name);

    if (file < 0)
        return -1;

    // Procura um descritor de arquivo disponível
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (!open_files[i].used)
        {
            open_files[i].used = 1;
            open_files[i].position = 0;
            open_files[i].dir_index = file;

            return i;
        }
    }

    // Limite de arquivos abertos atingido
    return -1;
}

int fs_close(int fd)
{
    // Verifica se o descritor é válido
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

    if (!open_files[fd].used)
        return -1;

    // Libera o descritor de arquivo
    open_files[fd].used = 0;
    return 0;
}

int fs_write(int fd,
             const void *buffer,
             uint32_t size)
{
    // Valida o descritor e o buffer
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

    if (!open_files[fd].used)
        return -1;

    if (buffer == NULL)
        return -1;

    int dir = open_files[fd].dir_index;

    // Obtém o primeiro cluster do arquivo
    uint16_t cluster =
        root_dir[dir].first_cluster;

    const uint8_t *src =
        (const uint8_t *)buffer;

    uint32_t written = 0;
    uint8_t block_buffer[BLOCK_SIZE];

    // Escreve os dados bloco a bloco
    while (written < size)
    {
        memset(block_buffer,
               0,
               BLOCK_SIZE);

        uint32_t bytes =
            size - written;

        if (bytes > BLOCK_SIZE)
            bytes = BLOCK_SIZE;

        memcpy(block_buffer,
               src + written,
               bytes);

        if (block_write(cluster,
                        block_buffer) < 0)
        {
            return -1;
        }

        written += bytes;

        // Aloca um novo cluster caso ainda existam dados a escrever
        if (written < size)
        {
            if (fat[cluster] == FAT_EOF)
            {
                int new_cluster =
                    cluster_alloc();

                if (new_cluster < 0)
                    return written;

                fat[cluster] = new_cluster;
                cluster = new_cluster;
            }
            else
            {
                // Continua na cadeia de clusters existente
                cluster = fat[cluster];
            }
        }
    }

    // Atualiza o tamanho do arquivo e a posição atual
    root_dir[dir].size = written;
    open_files[fd].position = written;

    return written;
}

int fs_read(int fd,
            void *buffer,
            uint32_t size)
{
    // Valida o descritor e o buffer
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

    if (!open_files[fd].used)
        return -1;

    if (buffer == NULL)
        return -1;

    int dir = open_files[fd].dir_index;

    // Evita ler além do tamanho do arquivo
    if (size > root_dir[dir].size)
        size = root_dir[dir].size;

    // Obtém o primeiro cluster do arquivo
    uint16_t cluster =
        root_dir[dir].first_cluster;

    uint8_t *dst =
        (uint8_t *)buffer;

    uint8_t block_buffer[BLOCK_SIZE];

    uint32_t read = 0;

    // Lê o arquivo percorrendo a cadeia de clusters
    while (cluster != FAT_EOF &&
           read < size)
    {
        if (block_read(cluster,
                       block_buffer) < 0)
        {
            return -1;
        }

        uint32_t bytes =
            size - read;

        if (bytes > BLOCK_SIZE)
            bytes = BLOCK_SIZE;

        memcpy(dst + read,
               block_buffer,
               bytes);

        read += bytes;

        if (fat[cluster] == FAT_EOF)
            break;

        cluster = fat[cluster];
    }

    // Atualiza a posição atual de leitura
    open_files[fd].position = read;

    return read;
}

int fs_delete(const char *name)
{
    // Localiza o arquivo no diretório
    int file = find_file(name);

    if (file < 0)
        return -1;

    // Libera todos os clusters ocupados pelo arquivo
    cluster_free_chain(
        root_dir[file].first_cluster);

    // Remove a entrada do diretório raiz
    memset(&root_dir[file],
           0,
           sizeof(dir_entry_t));

    return 0;
}
