CC = i686-elf-gcc
AS = i686-elf-as
ARCH = arch/i686

INCLUDE = -I  arch/i686  -I kernel
CCFLAGS = -std=gnu++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti $(INCLUDE) -Wno-write-strings
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc
OBJECTS = build/boot.o build/main.o build/vga.o build/io.o build/gdt.o build/descriptor.o

all: kernel

kernel: boot.o main.o vga.o io.o gdt.o descriptor.o
	$(CC) -T $(ARCH)/linker.ld -o ollios.bin $(LDFLAGS) $(OBJECTS)

boot.o:
	$(AS) $(ARCH)/boot.s -o build/boot.o

main.o:
	$(CC) -c kernel/main.cpp -o build/main.o $(CCFLAGS)

vga.o:
	$(CC) -c $(ARCH)/vga.cpp -o build/vga.o $(CCFLAGS)

io.o:
	$(CC) -c $(ARCH)/io.cpp -o build/io.o $(CCFLAGS)

gdt.o:
	$(CC) -c $(ARCH)/gdt.cpp -o build/gdt.o $(CCFLAGS)

descriptor.o:
	$(CC) -c $(ARCH)/descriptor.s -o build/descriptor.o $(CCFLAGS)	