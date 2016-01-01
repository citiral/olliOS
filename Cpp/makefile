#structure: folders never start with /, always end with /

CC = i686-elf-gcc
AS = i686-elf-as
ARCH = arch/i686/

ROOT = root/
OUTPUT=ollios.bin

INCLUDE = -I $(ROOT)usr/include
CCFLAGS = -std=gnu++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -fdiagnostics-color=auto -nostdlib
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc
LIBS = $(ROOT)/usr/lib/libk.a

KERNEL_CPP = $(wildcard kernel/*.cpp) $(wildcard $(ARCH)*.cpp)
KERNEL_ASM = $(wildcard kernel/*.s) $(wildcard $(ARCH)*.s)
HEADERS = $(wildcard kernel/*.h) $(wildcard $(ARCH)*.h)

CRTI_OBJ=crti.o
CRTN_OBJ=crtn.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CCFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CCFLAGS) -print-file-name=crtend.o)

#make a list of all objects, but taking special care of the order of crt* files
OBJECTS = $(filter-out crti.o crtn.o, $(notdir $(KERNEL_CPP:.cpp=.o)) $(notdir $(KERNEL_ASM:.s=.o)))

.PHONY: all kernel clean dir install install-headers install-kernel

all: dir install-headers kernel install-kernel

kernel: $(CRTI_OBJ) $(OBJECTS) $(CRTN_OBJ)
	$(CC) -T $(ARCH)linker.ld -o build/$(OUTPUT) $(CFLAGS) $(addprefix build/, $(CRTI_OBJ)) $(CRTBEGIN_OBJ) $(addprefix build/,$(OBJECTS)) $(CRTEND_OBJ) $(addprefix build/, $(CRTN_OBJ)) $(LDFLAGS) $(LIBS)

clean:
	rm -fr build root

%.o: kernel/%.cpp 
	$(CC) -c $< -o build/$@ $(CCFLAGS) 

%.o: kernel/%.s
	$(CC) -c $< -o build/$@ $(CCFLAGS)

%.o: $(ARCH)%.cpp 
	$(CC) -c $< -o build/$@ $(CCFLAGS)

%.o: $(ARCH)%.s
	$(CC) -c $< -o build/$@ $(CCFLAGS)

dir:
	mkdir -p build
	mkdir -p $(ROOT)
	mkdir -p $(ROOT)usr
	mkdir -p $(ROOT)usr
	mkdir -p $(ROOT)usr/include
	mkdir -p $(ROOT)usr/boot
	mkdir -p $(ROOT)usr/lib
	mkdir -p $(ROOT)usr/bin

install: install-headers install-kernel

install-headers:
	cp -Rv $(HEADERS) $(ROOT)usr/include

install-kernel:
	cp -RTv build/$(OUTPUT) $(ROOT)usr/boot/$(OUTPUT)