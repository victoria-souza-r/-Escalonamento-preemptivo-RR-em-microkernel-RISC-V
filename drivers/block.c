#include "block.h"
#include "string.h"
#include <stddef.h>

// Disco virtual
static uint8_t disk[NUM_BLOCKS][BLOCK_SIZE];

int block_read(uint32_t block, void *buffer)
{
    if (block >= NUM_BLOCKS || buffer == NULL)
        return -1;

    memcpy(buffer, disk[block], BLOCK_SIZE);
    return 0;
}

int block_write(uint32_t block, const void *buffer)
{
    if (block >= NUM_BLOCKS || buffer == NULL)
        return -1;

    memcpy(disk[block], buffer, BLOCK_SIZE);
    return 0;
}
