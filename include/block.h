#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

// Configuração do disco virtual 

#define BLOCK_SIZE     512
#define NUM_BLOCKS     1024

// Interface do driver de blocos 

int block_read(uint32_t block, void *buffer);
int block_write(uint32_t block, const void *buffer);

#endif
