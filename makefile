#structure: folders never start /, always end with /

CC = i686-elf-g++
AS = i686-elf-as
ARCH = arch/i686/
MAKE = make

BUILD = build/
ROOT = root/
OUTPUT=ollios.bin
ISO=ollios.iso

INCLUDE = -I $(ROOT)usr/include -I $(ROOT)usr/include/libk -I $(ROOT)include
CCFLAGS = -D__is_kernel -std=gnu++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -nostdlib -fno-threadsafe-statics -Werror=return-type -mgeneral-regs-only  -m32 -MD
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc

MODULES = keyboard shell pci ata mbr iso9660 sysint ramfs
APPS = hello_world echo cat

KERNEL_CPP = $(wildcard kernel/*.cpp) $(wildcard kernel/*/*.cpp)
KERNEL_C = $(wildcard kernel/libk/*/*.c)
KERNEL_ASM = $(wildcard kernel/*.s) $(wildcard kernel/*/*.s)
KERNEL_NASM = $(wildcard kernel/*.asm) $(wildcard kernel/*/*.asm)
HEADERS = $(wildcard kernel/*.h) $(wildcard kernel/*/*.h)

CRTI_OBJ=crti.o
CRTN_OBJ=crtn.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CCFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CCFLAGS) -print-file-name=crtend.o)

#make a list of all objects, but taking special care of the order of crt* files
OBJECTS = $(addprefix $(BUILD), $(filter-out crti.o crtn.o, $(notdir $(KERNEL_CPP:.cpp=.o)) $(notdir $(KERNEL_C:.c=.o)) $(notdir $(KERNEL_ASM:.s=.o))  $(notdir $(KERNEL_NASM:.asm=.o))))
DEPS = $(OBJECTS:.o=.d)

.PHONY: all newlib compile-kernel clean dir install install-headers $(MODULES) $(LIBS) $(APPS)

all: dir install-headers $(BUILD)$(OUTPUT) $(MODULES) $(LIBS) $(APPS) $(ISO)

-include $(DEPS)

dir: $(BUILD) $(ROOT) $(ROOT)boot $(ROOT)boot/grub $(ROOT)usr $(ROOT)usr/bin $(ROOT)usr/include $(ROOT)usr/lib

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
$(ROOT)usr/bin:
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
$(BUILD)%.o: kernel/**/%.s
	$(CC) -c $< -o $@ $(CCFLAGS)
$(BUILD)%.o: kernel/**/**/%.s
	$(CC) -c $< -o $@ $(CCFLAGS)
$(BUILD)%.o: kernel/%.asm
	nasm -felf32 $< -o $@
$(BUILD)%.o: kernel/**/%.asm
	nasm -felf32 $< -o $@
$(BUILD)%.o: kernel/**/**/%.asm
	nasm -felf32 $< -o $@

#$(ISO): install

install-headers: $(addprefix $(ROOT)usr/include/, $(HEADERS:kernel/%=%))
$(ISO): $(addprefix $(ROOT)usr/include/, $(HEADERS:kernel/%=%)) $(ROOT)boot/$(OUTPUT) $(ROOT)boot/ollios.sym $(ROOT)boot/$(OUPUT) $(ROOT)boot/grub/grub.cfg $(MODULES:%=$(ROOT)boot/%.so) $(LIBS:%=$(ROOT)usr/lib/%.a) $(APPS:%=$(ROOT)usr/bin/%)
	grub-mkrescue root -o ollios.iso


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

#compile apps
$(APPS):
	make -C apps APP=$@ all

#compile libs
$(LIBS):
	make -C libs LIB=$@ all

$(ROOT)boot/%.so: $(BUILD)%.so
	cp $^ $@

# newlib
newlib:
	bash -c "cd newlib/build; ./build.sh"
	cp newlib/build/out/i686-ollios/* $(ROOT) -r

# run an emulator
qemu: all
	./qemu.sh

bochs: all
	./bochs.sh
