#!/bin/bash

# start qemu with the iso loaded in a cdromdrive in ata device 0
qemu-system-i386 -s -drive file=ollios.iso,if=ide,index=0,media=cdrom -serial stdio -smp 4 -d guest_errors
