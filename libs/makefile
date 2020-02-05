#structure: folders never start /, always end with /
CC = i686-elf-g++
AS = i686-elf-as
ARCH = arch/i686/
MAKE = make

ROOT = ../root/
BUILD = ../build/
OUTDIR = $(BUILD)program/$(LIB)/

INCLUDE = -I $(ROOT)usr/include -I $(ROOT)usr/include/libk
CCFLAGS = -std=gnu++11 -ffreestanding -O0 -Wall -Wextra -fno-exceptions -fno-rtti $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -nostdlib -fno-threadsafe-statics -Werror=return-type -m32 -mgeneral-regs-only -MD
LDFLAGS = -std=gnu++11 -Wall -Wextra -nostdlib -m32

LIB_CPP = $(wildcard $(LIB)/*.cpp)
LIB_C = $(wildcard $(LIB)/*.c)
LIB_ASM = $(wildcard $(LIB)/*.asm)
HEADERS = $(wildcard $(LIB)/*.h)
OBJECTS = $(addprefix $(OUTDIR), $(notdir $(LIB_CPP:.cpp=.o)) $(notdir $(LIB_C:.c=.o)) $(notdir $(LIB_ASM:.asm=.o)))

DEPS = $(OBJECTS:.o=.d)


.phony: all clean install

all: $(ROOT)usr/lib/$(LIB).a install

clean:
	rm -fr $(OUTDIR)
	rm -f $(ROOT)usr/lib/$(LIB).a

install: $(ROOT)usr/lib/$(LIB).a install-headers

install-headers: $(addprefix $(ROOT)usr/include/, $(HEADERS))

$(ROOT)usr/lib/$(LIB).a: $(OUTDIR)$(LIB).a
	cp $^ $@

$(ROOT)usr/include/$(LIB)/%.h: $(LIB)/%.h
	mkdir $(shell dirname $@) -p
	cp $^ $@

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)$(LIB).a: $(OUTDIR) $(OBJECTS)
	ar rcs $@ $(OBJECTS)

$(OUTDIR)%.o: $(LIB)/%.cpp
	$(CC) -c $< -o $@ $(CCFLAGS)

$(OUTDIR)%.o: $(LIB)/%.c
	$(CC) -c $< -o $@ $(CCFLAGS)

$(OUTDIR)%.o: $(LIB)/%.asm
	nasm -felf32 $< -o $@

-include $(DEPS)