#!/bin/bash

#../newlib/configure --prefix=/usr --target=i686-ollios
make all
make DESTDIR=root install
cp i686-ollios/newlib/root/usr/i686-ollios/* ../root/ -r
