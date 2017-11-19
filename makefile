#structure: folders never start /, always end with /

CC = i686-elf-g++
AS = i686-elf-as
ARCH = arch/i686/
MAKE = make

ROOT = root/
OUTPUT=ollios.bin

INCLUDE = -I $(ROOT)usr/include
CCFLAGS = -D__is_kernel -std=gnu++11 -ffreestanding -O0 -Wall -Wextra -fno-exceptions -fno-rtti $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -nostdlib -fno-threadsafe-statics -Werror=return-type -m32 -g
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc
LIBS = $(ROOT)/usr/lib/libk.a

KERNEL_CPP = $(wildcard kernel/*.cpp) $(wildcard kernel/eventbus/*.cpp) $(wildcard kernel/util/*.cpp) $(wildcard kernel/devices/*.cpp) $(wildcard kernel/devices/ata/*.cpp) $(wildcard kernel/devices/pci/*.cpp) $(wildcard kernel/alloc/*.cpp) $(wildcard kernel/fs/*.cpp) $(wildcard kernel/kstd/*.cpp)  $(wildcard kernel/memory/*.cpp)
KERNEL_ASM = $(wildcard kernel/*.s)
HEADERS = $(wildcard kernel/*.h) $(wildcard kernel/eventbus/*.h) $(wildcard kernel/util/*.h) $(wildcard kernel/devices/*.h) $(wildcard kernel/devices/ata/*.h) $(wildcard kernel/devices/pci/*.h) $(wildcard kernel/alloc/*.h) $(wildcard kernel/fs/*.h) $(wildcard kernel/kstd/*.h)  $(wildcard kernel/memory/*.h)

CRTI_OBJ=crti.o
CRTN_OBJ=crtn.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CCFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CCFLAGS) -print-file-name=crtend.o)

#make a list of all objects, but taking special care of the order of crt* files
OBJECTS = $(filter-out crti.o crtn.o, $(notdir $(KERNEL_CPP:.cpp=.o)) $(notdir $(KERNEL_ASM:.s=.o)))

.PHONY: all compile-kernel clean dir libk install install-grub install-headers install-kernel iso

all: dir iso

libk:
	$(MAKE) -C libk

#libkpp:
#	$(MAKE) -C libkpp

kernel: dir install-headers compile-kernel install-kernel install-grub iso

compile-kernel: $(CRTI_OBJ) $(OBJECTS) $(CRTN_OBJ)
	$(CC) -T kernel/linker.ld -o build/$(OUTPUT) $(CFLAGS) $(addprefix build/, $(CRTI_OBJ)) $(CRTBEGIN_OBJ) $(addprefix build/,$(OBJECTS)) $(CRTEND_OBJ) $(addprefix build/, $(CRTN_OBJ)) $(LDFLAGS) $(LIBS)

clean:
	rm -fr build root
	rm ollios.iso

%.o: kernel/%.cpp
	$(CC) -c $< -o build/$@ $(CCFLAGS)

%.o: kernel/**/%.cpp
	$(CC) -c $< -o build/$@ $(CCFLAGS)

%.o: kernel/**/**/%.cpp
	$(CC) -c $< -o build/$@ $(CCFLAGS)

%.o: kernel/%.s
	$(CC) -c $< -o build/$@ $(CCFLAGS)

dir:
	mkdir -p build
	mkdir -p $(ROOT)
	mkdir -p $(ROOT)boot
	mkdir -p $(ROOT)boot/grub
	mkdir -p $(ROOT)usr
	mkdir -p $(ROOT)usr/include
	mkdir -p $(ROOT)usr/lib

iso: install
	mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o ollios.iso root

bootiso: install
	grub-mkrescue ./root -o ./ollios.iso

install: install-headers install-kernel install-grub

install-grub:
	cp -Rv grub/* $(ROOT)boot/grub

install-headers:
	cd kernel && cp --parents -v *.h  ../$(ROOT)usr/include/
	cd kernel && cp --parents -v **/*.h  ../$(ROOT)usr/include/
	cd kernel && cp --parents -v **/**/*.h  ../$(ROOT)usr/include/

install-kernel: libk compile-kernel
	cp -RTv build/$(OUTPUT) $(ROOT)boot/$(OUTPUT)

qemu: all
	./qemu.sh

bochs: all
	./bochs.sh