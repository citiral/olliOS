#!/bin/bash

#first compile the bootstrapper
i686-elf-ld -T linker.ld -o bin/ollios.bin -O2 -nostdlib --gc-sections bin/boot.o ../../bin/libkernel.a