#!/bin/bash

# start qemu with the iso loaded in a cdromdrive in ata device 0
qemu-system-i386 -vga std -gdb tcp::1234 -drive id=disk,file=ollios.iso,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0
