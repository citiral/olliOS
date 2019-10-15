#!/bin/bash

# start qemu with the iso loaded in a cdromdrive in ata device 0
qemu-system-i386 -smp 16 -gdb tcp::1234 -drive file=ollios.iso,if=ide,index=0,media=cdrom -serial stdio $1   #-drive file=disk.img,if=ide,media=disk
