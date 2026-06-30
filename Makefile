CROSS = riscv64-unknown-elf-

CC = $(CROSS)gcc
LD = $(CROSS)ld

CFLAGS = -march=rv64gc -mabi=lp64 \
         -mcmodel=medany \
         -msmall-data-limit=0 \
         -ffreestanding \
         -nostdlib -nostartfiles \
         -fno-stack-protector \
         -Wall -Iinclude -g

# Objetos do projeto
OBJS = start.o trap_entry.o context.o \
       main.o task.o scheduler.o uart.o string.o memory.o \
       timer.o trap.o fs.o block.o

all: kernel.elf

# Regras de compilação apontando para as pastas corretas
start.o: boot/start.S
	$(CC) $(CFLAGS) -c boot/start.S -o start.o

trap_entry.o: boot/trap_entry.S
	$(CC) $(CFLAGS) -c boot/trap_entry.S -o trap_entry.o

context.o: kernel/context.S
	$(CC) $(CFLAGS) -c kernel/context.S -o context.o

main.o: kernel/main.c
	$(CC) $(CFLAGS) -c kernel/main.c -o main.o

task.o: kernel/task.c
	$(CC) $(CFLAGS) -c kernel/task.c -o task.o

scheduler.o: kernel/scheduler.c
	$(CC) $(CFLAGS) -c kernel/scheduler.c -o scheduler.o

uart.o: kernel/uart.c
	$(CC) $(CFLAGS) -c kernel/uart.c -o uart.o

string.o: kernel/string.c
	$(CC) $(CFLAGS) -c kernel/string.c -o string.o

memory.o: kernel/memory.c
	$(CC) $(CFLAGS) -c kernel/memory.c -o memory.o

timer.o: kernel/timer.c
	$(CC) $(CFLAGS) -c kernel/timer.c -o timer.o

trap.o: kernel/trap.c
	$(CC) $(CFLAGS) -c kernel/trap.c -o trap.o

fs.o: kernel/fs.c
	$(CC) $(CFLAGS) -c kernel/fs.c -o fs.o

block.o: drivers/block.c
	$(CC) $(CFLAGS) -c drivers/block.c -o block.o

# Linkagem
kernel.elf: $(OBJS)
	$(CC) $(CFLAGS) -T linker.ld $(OBJS) -o kernel.elf

clean:
	rm -f *.o kernel.elf
