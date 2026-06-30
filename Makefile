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

all: kernel.elf
start.o:$(CROSS)gcc $(CFLAGS) -c boot/start.S
trap_entry.o:$(CROSS)gcc $(CFLAGS) -c boot/trap_entry.S
context.o:$(CROSS)gcc $(CFLAGS) -c kernel/context.S
main.o:$(CROSS)gcc $(CFLAGS) -c kernel/main.c
task.o:$(CROSS)gcc $(CFLAGS) -c kernel/task.c
scheduler.o:$(CROSS)gcc $(CFLAGS) -c kernel/scheduler.c
uart.o:$(CROSS)gcc $(CFLAGS) -c kernel/uart.c
string.o:$(CROSS)gcc $(CFLAGS) -c kernel/string.c
memory.o:$(CROSS)gcc $(CFLAGS) -c kernel/memory.c
timer.o:$(CROSS)gcc $(CFLAGS) -c kernel/timer.c
trap.o:$(CROSS)gcc $(CFLAGS) -c kernel/trap.c
fs.o:$(CROSS)gcc $(CFLAGS) -c kernel/fs.c
block.o:$(CROSS)gcc $(CFLAGS) -c drivers/block.c
kernel.elf: $(OBJS)$(CROSS)gcc $(CFLAGS) -T linker.ld $(OBJS) -o kernel.elf

clean:
rm -f *.o kernel.elf
