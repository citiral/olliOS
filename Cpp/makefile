CC = i686-elf-gcc
AS = i686-elf-as
ARCH = arch/i686

INCLUDE = -I  arch/i686  -I kernel
CCFLAGS = -std=gnu++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti $(INCLUDE) -Wno-write-strings
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc
OBJECTS = build/bootASM.o build/main.o build/vga.o build/io.o build/gdt.o build/descriptorASM.o build/interruptASM.o build/interrupt.o

all: kernel

kernel: bootASM.o main.o vga.o io.o gdt.o descriptorASM.o descriptor.o interruptASM.o interrupt.o
	$(CC) -T $(ARCH)/linker.ld -o ollios.bin $(LDFLAGS) $(OBJECTS)

bootASM.o:
	$(AS) $(ARCH)/boot.s -o build/bootASM.o

main.o:
	$(CC) -c kernel/main.cpp -o build/main.o $(CCFLAGS)

vga.o:
	$(CC) -c $(ARCH)/vga.cpp -o build/vga.o $(CCFLAGS)

io.o:
	$(CC) -c $(ARCH)/io.cpp -o build/io.o $(CCFLAGS)

gdt.o:
	$(CC) -c $(ARCH)/gdt.cpp -o build/gdt.o $(CCFLAGS)

descriptorASM.o:
	$(CC) -c $(ARCH)/descriptor.s -o build/descriptorASM.o $(CCFLAGS)

descriptor.o:
	$(CC) -c $(ARCH)/descriptor.cpp -o build/descriptor.o $(CCFLAGS)

interruptASM.o:
	$(CC) -c $(ARCH)/interrupt.s -o build/interruptASM.o $(CCFLAGS)

interrupt.o:
	$(CC) -c $(ARCH)/interrupt.cpp -o build/interrupt.o $(CCFLAGS)