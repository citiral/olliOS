#!/bin/bash

cd ./arch/i686/
./build.sh
cd ../../

rustc -O -Z no-landing-pads --cfg i686-unknown-linux-gnu --target=target.json -g --crate-type=staticlib --emit=link --out-dir ./bin/ ./kernel/kernel.rs --extern core=arch/i686/lib/libcore.rlib

cd ./arch/i686/
./link.sh
cd ../../