#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Interface do timer do sistema (usado para preempção)
void timer_init(uint64_t interval);
void timer_next(void);

#endif // finaliza a proteção do header
