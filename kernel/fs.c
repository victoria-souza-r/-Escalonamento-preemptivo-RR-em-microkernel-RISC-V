#include "fs.h"
#include "string.h"
#include <stddef.h>

/* Estruturas internas */

static superblock_t superblock;
static uint16_t fat[NUM_CLUSTERS];
static dir_entry_t root_dir[MAX_FILES];
static file_descriptor_t open_files[MAX_OPEN_FILES];


/* Funções auxiliares */

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

    return -1;
}

int cluster_alloc(void)
{
    /* cluster 0 reservado */

    for (uint16_t i = 1; i < NUM_CLUSTERS; i++)
    {
        if (fat[i] == FAT_FREE)
        {
            fat[i] = FAT_EOF;
            return i;
        }
    }

    return -1;
}

void cluster_free_chain(uint16_t cluster)
{
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

/* Sistema de arquivos       */
int fs_init(void)
{
    superblock.magic = FS_MAGIC;
    superblock.total_blocks = NUM_BLOCKS;
    superblock.total_clusters = NUM_CLUSTERS;
    superblock.cluster_size = CLUSTER_SIZE;

    memset(fat, 0, sizeof(fat));
    memset(root_dir, 0, sizeof(root_dir));
    memset(open_files, 0, sizeof(open_files));

    return 0;
}

int fs_create(const char *name)
{
    if (name == NULL)
        return -1;

    if (find_file(name) >= 0)
        return -1;

    int entry = -1;

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

    int cluster = cluster_alloc();

    if (cluster < 0)
        return -1;

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
    int file = find_file(name);

    if (file < 0)
        return -1;

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

    return -1;
}

int fs_close(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

    if (!open_files[fd].used)
        return -1;

    open_files[fd].used = 0;
    return 0;
}

int fs_write(int fd,
             const void *buffer,
             uint32_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

    if (!open_files[fd].used)
        return -1;

    if (buffer == NULL)
        return -1;

    int dir = open_files[fd].dir_index;

    uint16_t cluster =
        root_dir[dir].first_cluster;

    const uint8_t *src =
        (const uint8_t *)buffer;

    uint32_t written = 0;
    uint8_t block_buffer[BLOCK_SIZE];

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
                cluster = fat[cluster];
            }
        }
    }

    root_dir[dir].size = written;
    open_files[fd].position = written;

    return written;
}

int fs_read(int fd,
            void *buffer,
            uint32_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

    if (!open_files[fd].used)
        return -1;

    if (buffer == NULL)
        return -1;

    int dir = open_files[fd].dir_index;

    if (size > root_dir[dir].size)
        size = root_dir[dir].size;

    uint16_t cluster =
        root_dir[dir].first_cluster;

    uint8_t *dst =
        (uint8_t *)buffer;

    uint8_t block_buffer[BLOCK_SIZE];

    uint32_t read = 0;

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

    open_files[fd].position = read;

    return read;
}

int fs_delete(const char *name)
{
    int file = find_file(name);

    if (file < 0)
        return -1;

    cluster_free_chain(
        root_dir[file].first_cluster);

    memset(&root_dir[file],
           0,
           sizeof(dir_entry_t));

    return 0;
}
