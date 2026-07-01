.section .text
.global user_task_start

user_task_start:
    la a0, message     # Carrega o endereço da string no registrador a0

loop:
    li a7, 1           # Código da syscall (print)
    ecall              # Chama o kernel via trap (system call)
    j loop             # Loop infinito imprimindo a mensagem

.section .rodata
message:
    .string "Hello from User Mode!\n"
