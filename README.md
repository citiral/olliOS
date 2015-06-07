# olliOS
Custom operating system written in Rust as a learning project for both rust &amp; operating system development.

## Features

It shows some text, the screen can scroll and it can _sucessfully_ crash.

## Compiling

Currently some small shell scripts are in place to compile. It will eventually be possible to select the arch you want to use, but for now, just run `build.sh` and it should compile the OS for i386/i686
To compile the rust kernel, libcore first needs to be compiled using the target.json provided in the arch you want and the library should be placed in the lib folder of said arch. To make the process easier, a precompiled binary is already present in the repository.

## Running

The operating system can be started using multiboot, so it can be booted using most bootloaders. If you just want to test it, it can run perfectly in qemu. There is even a script called `run.sh` that does jus that.

## Contributing

I don't expect anyone to commit to this project, since this is more of a personal learning experience, but if you really want to, be my guest! Just make a pull request for your changes and I will examine it as soon as possible
