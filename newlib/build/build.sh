#!/bin/bash

ln -sf /usr/bin/i686-elf-ar i686-ollios-ar
ln -sf /usr/bin/i686-elf-as i686-ollios-as
ln -sf /usr/bin/i686-elf-gcc i686-ollios-gcc
ln -sf /usr/bin/i686-elf-gcc i686-ollios-cc
ln -sf /usr/bin/i686-elf-ranlib i686-ollios-ranlib

PATH=$(pwd):$PATH

if [ ! -f ./config.status ]; then
    ../source/configure --prefix=$(pwd)/out --target=i686-ollios
fi

make all
make install