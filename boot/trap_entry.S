section .text
.global trap_entry
.align 4

trap_entry:
    # Abre espaço na pilha da tarefa atual para guardar 31 registradores (31 * 8 = 248, arredondado para 256 bytes)
    addi sp, sp, -256

    # SALVA O CONTEXTO
    sd ra,   0(sp)     # frame[0] -> regs[0] (ra)
    sd sp,   8(sp)     # frame[1] -> regs[1] (sp) 
    sd gp,   16(sp)    # frame[2] -> regs[2]
    sd tp,   24(sp)    # frame[3] -> regs[3]
    sd t0,   32(sp)    # frame[4] -> regs[4]
    sd t1,   40(sp)
    sd t2,   48(sp)
    sd s0,   56(sp)
    sd s1,   64(sp)
    sd a0,   72(sp)
    sd a1,   80(sp)
    sd a2,   88(sp)
    sd a3,   96(sp)
    sd a4,   104(sp)
    sd a5,   112(sp)
    sd a6,   120(sp)
    sd a7,   128(sp)
    sd s2,   136(sp)
    sd s3,   144(sp)
    sd s4,   152(sp)
    sd s5,   160(sp)
    sd s6,   168(sp)
    sd s7,   176(sp)
    sd s8,   184(sp)
    sd s9,   192(sp)
    sd s10,  200(sp)
    sd s11,  208(sp)
    sd t3,   216(sp)
    sd t4,   224(sp)
    sd t5,   232(sp)
    sd t6,   240(sp)   # frame[30] -> regs[30]

    # O valor real do sp antes do trap era (sp atual + 256)
    addi t0, sp, 256
    sd t0, 8(sp)       # Atualiza o frame[1] com o valor correto da pilha da tarefa

    # Envia o topo da pilha (ponteiro do frame) como parâmetro em a0 para o C
    mv a0, sp
    call trap_handler

    # RESTAURA O CONTEXTO DA NOVA TAREFA ESCOLHIDA PELO SCHEDULER
    ld ra,   0(sp)
    # Não restaura o sp (8(sp)) agora para não perder o ponteiro do frame antes da hora
    ld gp,   16(sp)
    ld tp,   24(sp)
    ld t0,   32(sp)
    ld t1,   40(sp)
    ld t2,   48(sp)
    ld s0,   56(sp)
    ld s1,   64(sp)
    ld a0,   72(sp)
    ld a1,   80(sp)
    ld a2,   88(sp)
    ld a3,   96(sp)
    ld a4,   104(sp)
    ld a5,   112(sp)
    ld a6,   120(sp)
    ld a7,   128(sp)
    ld s2,   136(sp)
    ld s3,   144(sp)
    ld s4,   152(sp)
    ld s5,   160(sp)
    ld s6,   168(sp)
    ld s7,   176(sp)
    ld s8,   184(sp)
    ld s9,   192(sp)
    ld s10,  200(sp)
    ld s11,  208(sp)
    ld t3,   216(sp)
    ld t4,   224(sp)
    ld t5,   232(sp)
    ld t6,   240(sp)

    # Restaura por fim a pilha (sp) da nova tarefa que o scheduler escolheu
    ld sp,   8(sp)

    # Retorna do Trap ligando o hardware novamente na nova tarefa
    sret
