.section .text
.align 4
.global trap_entry

trap_entry:
    # Aloca exatamente 256 bytes na pilha para o frame de registros 
    addi sp, sp, -256

    # Como alteramos o SP, precisamos salvar o SP original da Task.
    # Primeiro, salvamos o x31 no seu local definitivo (índice 30 -> offset 240) 
    sd x31, 240(sp)

    # Usamos o x31 temporariamente para calcular o SP original (SP atual + 256)
    addi x31, sp, 256
    sd x31, 8(sp)      # Salva o SP original no slot do x2 (índice 1 -> offset 8)

    # Salva os demais 29 registradores mapeando direto do índice 0 ao 30 do C 
    sd x1, 0(sp)       # frame[0]  -> x1 (ra)
    # x2 (sp) já foi calculado e salvo acima no 8(sp)
    sd x3, 16(sp)      # frame[2]  -> x3
    sd x4, 24(sp)      # frame[3]  -> x4
    sd x5, 32(sp)      # frame[4]  -> x5
    sd x6, 40(sp)      # frame[5]  -> x6
    sd x7, 48(sp)      # frame[6]  -> x7
    sd x8, 56(sp)      # frame[7]  -> x8
    sd x9, 64(sp)      # frame[8]  -> x9
    sd x10, 72(sp)     # frame[9]  -> x10 (a0)
    sd x11, 80(sp)     # frame[10] -> x11
    sd x12, 88(sp)     # frame[11] -> x12
    sd x13, 96(sp)     # frame[12] -> x13
    sd x14, 104(sp)    # frame[13] -> x14
    sd x15, 112(sp)    # frame[14] -> x15
    sd x16, 120(sp)    # frame[15] -> x16
    sd x17, 128(sp)    # frame[16] -> x17
    sd x18, 136(sp)    # frame[17] -> x18
    sd x19, 144(sp)    # frame[18] -> x19
    sd x20, 152(sp)    # frame[19] -> x20
    sd x21, 160(sp)    # frame[20] -> x21
    sd x22, 168(sp)    # frame[21] -> x22
    sd x23, 176(sp)    # frame[22] -> x23
    sd x24, 184(sp)    # frame[23] -> x24
    sd x25, 192(sp)    # frame[24] -> x25
    sd x26, 200(sp)    # frame[25] -> x26
    sd x27, 208(sp)    # frame[26] -> x27
    sd x28, 216(sp)    # frame[27] -> x28
    sd x29, 224(sp)    # frame[28] -> x29
    sd x30, 232(sp)    # frame[29] -> x30
    # x31 já foi salvo no offset 240(sp) no início

    # Passa o endereço atual do SP (o ponteiro do frame) para o argumento a0 
    mv a0, sp
    call trap_handler 

    # Restaura os registradores (que agora contêm o contexto da NOVA task) 
    ld x1, 0(sp)
    ld x3, 16(sp)
    ld x4, 24(sp)
    ld x5, 32(sp)
    ld x6, 40(sp)
    ld x7, 48(sp)
    ld x8, 56(sp)
    ld x9, 64(sp)
    ld x10, 72(sp)
    ld x11, 80(sp)
    ld x12, 88(sp)
    ld x13, 96(sp)
    ld x14, 104(sp)
    ld x15, 112(sp)
    ld x16, 120(sp)
    ld x17, 128(sp)
    ld x18, 136(sp)
    ld x19, 144(sp)
    ld x20, 152(sp)
    ld x21, 160(sp)
    ld x22, 168(sp)
    ld x23, 176(sp)
    ld x24, 184(sp)
    ld x25, 192(sp)
    ld x26, 200(sp)
    ld x27, 208(sp)
    ld x28, 216(sp)
    ld x29, 224(sp)
    ld x30, 232(sp)
    ld x31, 240(sp)
    
    ld sp, 8(sp)
    sret
