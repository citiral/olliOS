cd ./arch/i686/
call build.bat
cd ..
cd ..

rustc.exe  -O -Z no-landing-pads --cfg i686-unknown-linux-gnu --target=target.json -g --crate-type=staticlib --emit=link --out-dir=.\bin ./kernel/kernel.rs --extern core=bin/libcore.rlib

cd ./arch/i686/
call link.bat
cd ..
cd ..