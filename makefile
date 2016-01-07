#structure: folders never start /, always end with /

CC = i686-elf-gcc
AS = i686-elf-as
ARCH = arch/i686/
MAKE = make

ROOT = root/
OUTPUT=ollios.bin

INCLUDE = -I $(ROOT)usr/include
CCFLAGS = -D__is_kernel -std=gnu++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -nostdlib -fno-threadsafe-statics
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc
LIBS = $(ROOT)/usr/lib/libk.a #$(ROOT)/usr/lib/libk++.a

KERNEL_CPP = $(wildcard kernel/*.cpp) $(wildcard kernel/util/*.cpp) $(wildcard kernel/streams/*.cpp) $(wildcard $(ARCH)/*.cpp)  $(wildcard $(ARCH)streams/*.cpp)
KERNEL_ASM = $(wildcard kernel/*.s) $(wildcard $(ARCH)/*.s)
HEADERS = $(wildcard kernel/*.h) $(wildcard kernel/util/*.h) $(wildcard kernel/streams/*.h) $(wildcard $(ARCH)/*.h) $(wildcard $(ARCH)streams/*.h)

CRTI_OBJ=crti.o
CRTN_OBJ=crtn.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CCFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CCFLAGS) -print-file-name=crtend.o)

#make a list of all objects, but taking special care of the order of crt* files
OBJECTS = $(filter-out crti.o crtn.o, $(notdir $(KERNEL_CPP:.cpp=.o)) $(notdir $(KERNEL_ASM:.s=.o)))

.PHONY: all compile-kernel clean dir libk install install-headers install-kernel

all: dir install-headers libk compile-kernel install-kernel

libk:
	$(MAKE) -C libk

kernel: dir install-headers compile-kernel install-kernel

compile-kernel: $(CRTI_OBJ) $(OBJECTS) $(CRTN_OBJ)
	$(CC) -T $(ARCH)linker.ld -o build/$(OUTPUT) $(CFLAGS) $(addprefix build/, $(CRTI_OBJ)) $(CRTBEGIN_OBJ) $(addprefix build/,$(OBJECTS)) $(CRTEND_OBJ) $(addprefix build/, $(CRTN_OBJ)) $(LDFLAGS) $(LIBS)

clean:
	rm -fr build root

%.o: kernel/%.cpp
	$(CC) -c $< -o build/$@ $(CCFLAGS)

%.o: kernel/**/%.cpp
	$(CC) -c $< -o build/$@ $(CCFLAGS)

%.o: kernel/%.s
	$(CC) -c $< -o build/$@ $(CCFLAGS)

%.o: $(ARCH)%.cpp
	$(CC) -c $< -o build/$@ $(CCFLAGS)

%.o: $(ARCH)**/%.cpp
	$(CC) -c $< -o build/$@ $(CCFLAGS)

%.o: $(ARCH)%.s
	$(CC) -c $< -o build/$@ $(CCFLAGS)

dir:
	mkdir -p build
	mkdir -p $(ROOT)
	mkdir -p $(ROOT)boot
	mkdir -p $(ROOT)usr
	mkdir -p $(ROOT)usr/include
	mkdir -p $(ROOT)usr/lib
	mkdir -p $(ROOT)usr/bin

install: install-headers install-kernel

install-headers:
	cp -Rv $(HEADERS) $(ROOT)usr/include

install-kernel:
	cp -RTv build/$(OUTPUT) $(ROOT)boot/$(OUTPUT)
