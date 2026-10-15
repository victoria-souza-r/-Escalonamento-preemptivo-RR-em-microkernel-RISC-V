# Microkernel RISC-V com Escalonamento Round-Robin

Projeto acadêmico desenvolvido em C e Assembly para explorar conceitos de sistemas operacionais e arquitetura de computadores.

## Funcionalidades
- Escalonamento de tarefas com Round-Robin.
- Tratamento de interrupções de temporizador.
- Gerenciamento de contexto das tarefas.
- Sistema de arquivos SimpleFAT.
- Disco virtual para leitura e escrita de dados.

## Tecnologias
- C
- Assembly RISC-V
- GNU Make
- Toolchain RISC-V 64 bits

## Compilação

Requisitos: GNU Make e toolchain `riscv64-unknown-elf`.

```bash
make
```

Para limpar os arquivos gerados:

```bash
make clean
```

O comando `make` gera o arquivo `kernel.elf`. A execução em emulador ou hardware depende de um ambiente compatível.

## Objetivo

Aplicar conceitos de baixo nível, escalonamento de tarefas, interrupções e sistemas de arquivos no desenvolvimento de um kernel.
