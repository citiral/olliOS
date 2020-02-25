#!/bin/bash


#alias "i686-ollios-gcc=i686-elf-gcc -m32"
#i686-ollios-gcc

#ln -s  /usr/bin/i686-elf-ar i686-ollios-ar
#ln -s  /usr/bin/i686-elf-as i686-ollios-as
#ln -s  /usr/bin/i686-elf-gcc i686-ollios-gcc
#ln -s  /usr/bin/i686-elf-gcc i686-ollios-cc
#ln -s  /usr/bin/i686-elf-ranlib i686-ollios-ranlib

../newlib/newlib/configure --prefix=/usr --target=i686-ollios
make all
make DESTDIR=../root install
