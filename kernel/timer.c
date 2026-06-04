#include "timer.h"
#include <stdint.h>

static uint64_t tick_interval = 100000;

/* Implementa chamada SBI set_timer */
static inline void sbi_set_timer(uint64_t time) {
    register uint64_t a0 asm("a0") = time;
    register uint64_t a7 asm("a7") = 0; // 0x00 é o ID da extensão legacy set_timer
    asm volatile ("ecall" : : "r"(a0), "r"(a7) : "memory");
}
void timer_next(void) {
    uint64_t now;
    
    /* Le o CSR time */
    asm volatile("csrr %0, time" : "=r" (now));
    
    /* Programa now + tick_interval */
    sbi_set_timer(now + tick_interval);
}
void timer_init(uint64_t interval) {
    if (interval != 0) {
        tick_interval = interval;
    }
    timer_next();
    
    /* Habilita STIE no CSR sie (bit 5) */
    asm volatile("csrs sie, %0" : : "r" (1 << 5));
    
    /* Habilita SIE global em sstatus (bit 1) */
    asm volatile("csrs sstatus, %0" : : "r" (1 << 1));
} 
