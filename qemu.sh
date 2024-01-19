#!/bin/bash

# start qemu with the iso loaded in a cdromdrive in ata device 0
qemu-system-i386 \
    -curses \
    -gdb tcp::1234 \
    -cdrom ollios.iso \
    -drive file=disk.bin
