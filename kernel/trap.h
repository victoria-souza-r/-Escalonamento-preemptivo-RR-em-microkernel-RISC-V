#ifndef TRAP_H
#define TRAP_H

#include <stdint.h>

// Handler principal de traps (interrupções e exceções)
void trap_handler(uint64_t *frame);

#endif 
