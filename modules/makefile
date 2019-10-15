#structure: folders never start /, always end with /

CC = i686-elf-g++
AS = i686-elf-as
ARCH = arch/i686/
MAKE = make

ROOT = ../root/
BUILD = ../build/
OUTDIR = $(BUILD)$(MODULE)/

INCLUDE = -I $(ROOT)usr/include
CCFLAGS = -D__is_kernel -std=gnu++11 -ffreestanding -O0 -Wall -Wextra -fno-exceptions -fno-rtti $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -nostdlib -fno-threadsafe-statics -Werror=return-type -m32 -g -mgeneral-regs-only -fpic -Xassembler '-mrelax-relocations=no'
LDFLAGS =  -std=gnu++11 -Wall -Wextra -nostdlib -m32 -fpic -Xassembler '-mrelax-relocations=no' -r
LIBS = $(ROOT)/usr/lib/libk.a

MODULE_CPP = $(wildcard $(MODULE)/*.cpp)
OBJECTS = $(addprefix $(OUTDIR), $(notdir $(MODULE_CPP:.cpp=.o)))

.phony: all clean

all: $(OUTDIR) $(BUILD)$(MODULE).so

clean:
	rm -fr $(OUTDIR)
	rm ollios.iso

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(BUILD)$(MODULE).so: $(OBJECTS)
	$(CC) -fpic -o $@ $^ $(LDFLAGS) 

$(OUTDIR)%.o: $(MODULE)/%.cpp
	$(CC) -c $< -o $@ $(CCFLAGS)