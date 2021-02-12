#!/usr/bin/bash

#ln -sf $(pwd)/cross/bin/i686-elf-ar cross/bin/i686-ollios-ar
#ln -sf $(pwd)/cross/bin/i686-elf-as cross/bin/i686-ollios-as
#ln -sf $(pwd)/cross/bin/i686-elf-gcc cross/bin/i686-ollios-gcc
#ln -sf $(pwd)/cross/bin/i686-elf-gcc cross/bin/i686-ollios-cc
#ln -sf $(pwd)/cross/bin/i686-elf-ranlib cross/bin/i686-ollios-ranlib


export PATH="$(pwd)/cross/bin:$PATH"
