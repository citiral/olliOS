#!/bin/bash

umount /dev/loop0

#first, create and mount the image
dd if=/dev/zero of=./ollios.img bs=512 count=5760
losetup /dev/loop0 ./ollios.img
mkfs -t ext2 /dev/loop0

#install it
cp -r root/* ./mnt
grub-install --boot-directory=/mnt/boot /dev/loop0
