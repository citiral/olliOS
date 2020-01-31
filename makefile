#structure: folders never start /, always end with /

CC = i686-elf-g++
AS = i686-elf-as
ARCH = arch/i686/
MAKE = make

BUILD = build/
ROOT = root/
OUTPUT=ollios.bin
ISO=ollios.iso

INCLUDE = -I $(ROOT)usr/include -I $(ROOT)usr/include/libk
CCFLAGS = -D__is_kernel -std=gnu++11 -ffreestanding -O0 -Wall -Wextra -fno-exceptions -fno-rtti $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -nostdlib -fno-threadsafe-statics -Werror=return-type -m32 -g -mgeneral-regs-only -MD
LDFLAGS = -ffreestanding -O0 -nostdlib -lgcc
#LIBS = $(ROOT)/usr/lib/libk.a

MODULES = keyboard shell vga pci ata mbr

KERNEL_CPP = $(wildcard kernel/*.cpp) $(wildcard kernel/*/*.cpp)
KERNEL_C = $(wildcard kernel/libk/*/*.c)
KERNEL_ASM = $(wildcard kernel/*.s)
KERNEL_NASM = $(wildcard kernel/*.asm) $(wildcard kernel/threading/*.asm)
HEADERS = $(wildcard kernel/*.h) $(wildcard kernel/*/*.h)

CRTI_OBJ=crti.o
CRTN_OBJ=crtn.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CCFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CCFLAGS) -print-file-name=crtend.o)

#make a list of all objects, but taking special care of the order of crt* files
OBJECTS = $(addprefix $(BUILD), $(filter-out crti.o crtn.o, $(notdir $(KERNEL_CPP:.cpp=.o)) $(notdir $(KERNEL_C:.c=.o)) $(notdir $(KERNEL_ASM:.s=.o))  $(notdir $(KERNEL_NASM:.asm=.o))))
DEPS = $(OBJECTS:.o=.d)

.PHONY: all compile-kernel clean dir install install-headers $(MODULES)

all: dir install-headers $(BUILD)$(OUTPUT) $(MODULES) $(ISO)

-include $(DEPS)
	
dir: $(BUILD) $(ROOT) $(ROOT)boot $(ROOT)boot/grub $(ROOT)usr $(ROOT)usr/include $(ROOT)usr/lib

$(BUILD):
	mkdir -p $@
$(ROOT):
	mkdir -p $@
$(ROOT)boot:
	mkdir -p $@
$(ROOT)boot/grub:
	mkdir -p $@
$(ROOT)usr:
	mkdir -p $@
$(ROOT)usr/include:
	mkdir -p $@
$(ROOT)usr/lib:
	mkdir -p $@

libk:
	$(MAKE) -C libk

kernel: install-headers compile-kernel install-kernel install-grub iso

$(BUILD)$(OUTPUT): $(BUILD)$(CRTI_OBJ) $(OBJECTS) $(BUILD)$(CRTN_OBJ)
	$(CC) -T kernel/linker.ld -o build/$(OUTPUT) $(CFLAGS) $(addprefix build/, $(CRTI_OBJ)) $(CRTBEGIN_OBJ) $(OBJECTS) $(CRTEND_OBJ) $(addprefix build/, $(CRTN_OBJ)) $(LDFLAGS)

$(BUILD)ollios.sym: $(BUILD)$(OUTPUT)
	nm $^ | sort > $@

clean:
	rm -fr build root
	rm ollios.iso

$(BUILD)%.o: kernel/%.cpp
	$(CC) -c $< -o $@ $(CCFLAGS)
$(BUILD)%.o: kernel/**/%.cpp
	$(CC) -c $< -o $@ $(CCFLAGS)
$(BUILD)%.o: kernel/**/**/%.cpp
	$(CC) -c $< -o $@ $(CCFLAGS)
$(BUILD)%.o: kernel/%.c
	$(CC) -c $< -o $@ $(CCFLAGS)
$(BUILD)%.o: kernel/**/%.c
	$(CC) -c $< -o $@ $(CCFLAGS)
$(BUILD)%.o: kernel/**/**/%.c
	$(CC) -c $< -o $@ $(CCFLAGS)
$(BUILD)%.o: kernel/%.s
	$(CC) -c $< -o $@ $(CCFLAGS)

$(BUILD)%.o: kernel/%.asm
	nasm -felf32 $< -o $@
$(BUILD)%.o: kernel/**/%.asm
	nasm -felf32 $< -o $@

#$(ISO): install

install-headers: $(addprefix $(ROOT)usr/include/, $(HEADERS:kernel/%=%))
$(ISO): $(addprefix $(ROOT)usr/include/, $(HEADERS:kernel/%=%)) $(ROOT)boot/$(OUTPUT) $(ROOT)boot/ollios.sym $(ROOT)boot/$(OUPUT) $(ROOT)boot/grub/menu.lst $(ROOT)boot/grub/stage2_eltorito $(MODULES:%=$(ROOT)boot/%.so)
	mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $@ root


# installs files to root
$(ROOT)usr/include/%.h: kernel/%.h
	mkdir $(shell dirname $@) -p
	cp $^ $@
$(ROOT)boot/$(OUTPUT): $(BUILD)$(OUTPUT)
	mkdir $(shell dirname $@) -p
	cp $^ $@
$(ROOT)boot/ollios.sym: $(BUILD)ollios.sym
	mkdir $(shell dirname $@) -p
	cp $^ $@
$(ROOT)boot/grub/%: grub/%
	mkdir $(shell dirname $@) -p
	cp $^ $@


#compile modules
$(MODULES):
	make -C modules MODULE=$@ all

$(ROOT)boot/%.so: $(BUILD)%.so
	cp $^ $@


# run an emulator
qemu: all
	./qemu.sh

bochs: all
	./bochs.sh
