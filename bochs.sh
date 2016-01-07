#!/bin/bash

umount /dev/loop

#first, create and mount the image
kpartx -a ollios.img
mkfs.ext4 /dev/loop0
losetup /dev/loop0 ollios.img
mkfs -t ext2 /dev/loop0

#install it
cp -r root/* /mnt
grub-install --boot-directory=/mnt/boot /dev/loop0
