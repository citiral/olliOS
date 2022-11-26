#structure: folders never start /, always end with /

CC = i686-elf-gcc
AS = i686-elf-as
ARCH = arch/i686/
MAKE = make

ROOT = ../root/
BUILD = ../build/
OUTDIR = $(BUILD)program/$(APP)/

INCLUDE = -I $(ROOT)include -I $(ROOT)usr/include/
CCFLAGS = -D__is_kernel -O2 -Wall -Wextra -fno-exceptions $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -Werror=return-type -m32 -mgeneral-regs-only -MD --sysroot=$(ROOT)
LDFLAGS =  -std=gnu++11 -Wall -Wextra -O2 -m32 -L$(ROOT)lib --sysroot=$(ROOT)


APP_CPP = $(wildcard $(APP)/*.cpp)
APP_C = $(wildcard $(APP)/*.c)
APP_ASM = $(wildcard $(APP)/*.asm)
HEADERS = $(wildcard $(APP)/*.h)
OBJECTS = $(addprefix $(OUTDIR), $(notdir $(APP_CPP:.cpp=.o)) $(notdir $(APP_C:.c=.o)) $(notdir $(APP_ASM:.asm=.o)))

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
$(OUTDIR)%.o: $(APP)/%.c
	$(CC) -c $< -o $@ $(CCFLAGS)

$(OUTDIR)%.o: $(APP)/%.asm
	nasm -felf32 $< -o $@

-include $(DEPS)