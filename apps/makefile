#structure: folders never start /, always end with /

CC = i686-elf-g++
AS = i686-elf-as
ARCH = arch/i686/
MAKE = make

ROOT = ../root/
BUILD = ../build/
OUTDIR = $(BUILD)program/$(APP)/

INCLUDE = -I $(ROOT)usr/include -I $(ROOT)usr/include/libk
CCFLAGS = -D__is_kernel -std=gnu++11 -ffreestanding -O0 -Wall -Wextra -fno-exceptions -fno-rtti $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -nostdlib -fno-threadsafe-statics -Werror=return-type -m32 -mgeneral-regs-only -MD
LDFLAGS =  -std=gnu++11 -Wall -Wextra -nostdlib -m32 -L$(ROOT)usr/lib -lc


APP_CPP = $(wildcard $(APP)/*.cpp)
APP_ASM = $(wildcard $(APP)/*.asm)
HEADERS = $(wildcard $(APP)/*.h)
OBJECTS = $(addprefix $(OUTDIR), $(notdir $(APP_CPP:.cpp=.o)) $(notdir $(APP_ASM:.asm=.o)))

DEPS = $(OBJECTS:.o=.d)


.phony: all clean install

all: $(ROOT)usr/bin/$(APP) install

clean:
	rm -fr $(OUTDIR)
	rm -f $(ROOT)usr/bin/$(APP)

install: $(ROOT)usr/bin/$(APP)

$(ROOT)usr/bin/$(APP): $(OUTDIR)$(APP)
	cp $^ $@
	
$(ROOT)usr/include/$(APP)/%.h: $(APP)/%.h
	mkdir $(shell dirname $@) -p
	cp $^ $@

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)$(APP): $(OUTDIR) $(OBJECTS) linker.ld
	$(CC) -T linker.ld -o $@ $(OBJECTS) $(LDFLAGS)

$(OUTDIR)%.o: $(APP)/%.cpp
	$(CC) -c $< -o $@ $(CCFLAGS)

$(OUTDIR)%.o: $(APP)/%.asm
	nasm -felf32 $< -o $@

-include $(DEPS)