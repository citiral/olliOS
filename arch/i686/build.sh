#!/bin/bash

#first compile the bootstrapper
#nasm -w+orphan-labels -f elf32 -o ./bin/boot.old ./boot.asm
i686-elf-as ./boot.s -o ./bin/boot.o
i686-elf-as ./descriptor.s -o ./bin/descriptor.o
i686-elf-as ./interrupt.s -o ./bin/interrupt.o