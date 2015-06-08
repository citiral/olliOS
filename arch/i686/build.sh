#!/bin/bash

#first compile the bootstrapper
#nasm -f elf -o ./bin/boot.o ./boot.asm
i686-elf-as ./boot.s -o ./bin/boot.o
i686-elf-as ./descriptor.s -o ./bin/descriptor.o
i686-elf-as ./interrupt.s -o ./bin/interrupt.o