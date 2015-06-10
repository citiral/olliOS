# olliOS
Custom operating system written in Rust as a learning project for both rust &amp; operating system development.

## Features

It shows some text, the screen can scroll and it can _successfully_ crash.

## TODO

* Refactor initialisation
* Handling all interrupts
* Full fledged Keyboard wrapper

## Compiling

Currently some small shell scripts are in place to compile. It will eventually be possible to select the arch you want to use, but for now, just run `build.sh` and it should compile the OS for i386/i686.

## Running

The operating system can be started using multiboot, so it can be booted using most bootloaders. If you just want to test it, it can run perfectly in qemu. There is even a script called `run.sh` that does just that.

## Contributing

I don't expect anyone to commit to this project, since this is more of a personal learning experience, but if you really want to, be my guest! Just make a pull request for your changes and I will examine it as soon as possible.
