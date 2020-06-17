#!/usr/bin/bash -xe

make="make"

target=i686-elf
prefix=$(pwd)/cross

gcc_version=10.1.0
binutils_version=2.34

gcc_zip=gcc-$gcc_version.tar.xz
binutils_zip=binutils-$binutils_version.tar.xz

gcc_source=gcc-$gcc_version
binutils_source=binutils-$binutils_version

gcc_build=gcc-build
binutils_build=binutils_build

# make build folder
if [ ! -e env ]; then
  mkdir env
fi
cd env
if [ ! -e cross ]; then
  mkdir cross
fi

# download files
if [ ! -e $gcc_zip ]; then
  wget "https://ftp.gnu.org/gnu/gcc/gcc-10.1.0/$gcc_zip"
fi

if [ ! -e $binutils_zip ]; then
  wget "https://ftp.gnu.org/gnu/binutils/$binutils_zip"
fi

# extract 
if [ ! -e $gcc_source ]; then
  tar -xf $gcc_zip
fi

if [ ! -e $binutils_source ]; then
  tar -xf $binutils_zip
fi

# build
if [ ! -e $gcc_build ]; then
  mkdir $gcc_build
fi

if [ ! -e $binutils_build ]; then
  mkdir $binutils_build
fi

# build binutils
cd $binutils_build
if [ ! -e Makefile ]; then
  ../$binutils_source/configure --target=$target --prefix=$prefix --with-sysroot --disable-nls --disable-werror
fi
$make
$make install

# build gcc
cd ../$gcc_build
if [ ! -e Makefile ]; then
  ../$gcc_source/configure --target=$target --prefix=$prefix --disable-nls --enable-languages=c,c++ --without-headers
fi
$make all-gcc all-target-libgcc
$make install-gcc install-target-libgcc
