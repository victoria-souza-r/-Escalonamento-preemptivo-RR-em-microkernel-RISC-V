CROSS = riscv64-unknown-elf-

CFLAGS = -march=rv64gc -mabi=lp64 \
         -mcmodel=medany \
         -msmall-data-limit=0 \
         -ffreestanding \
         -nostdlib -nostartfiles \
         -fno-stack-protector \
         -Wall -Iinclude

# Objetos do projeto
OBJS = start.o trap_entry.o context.o \
       main.o task.o scheduler.o uart.o string.o memory.o \
       timer.o trap.o fs.o block.o

all:
	$(CROSS)gcc $(CFLAGS) -c boot/start.S
	$(CROSS)gcc $(CFLAGS) -c boot/trap_entry.S
	$(CROSS)gcc $(CFLAGS) -c kernel/context.S

	$(CROSS)gcc $(CFLAGS) -c kernel/main.c
	$(CROSS)gcc $(CFLAGS) -c kernel/task.c
	$(CROSS)gcc $(CFLAGS) -c kernel/scheduler.c
	$(CROSS)gcc $(CFLAGS) -c kernel/uart.c
	$(CROSS)gcc $(CFLAGS) -c kernel/string.c
	$(CROSS)gcc $(CFLAGS) -c kernel/memory.c
	$(CROSS)gcc $(CFLAGS) -c kernel/timer.c
	$(CROSS)gcc $(CFLAGS) -c kernel/trap.c

	# Novos módulos
	$(CROSS)gcc $(CFLAGS) -c kernel/fs.c
	$(CROSS)gcc $(CFLAGS) -c drivers/block.c

	# Linkedição
	$(CROSS)gcc $(CFLAGS) -T linker.ld $(OBJS) -o kernel.elf

clean:
	rm -f *.o kernel.elf
