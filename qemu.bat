D:
cd D:\Tools\qemu
dir
copy "\\wsl$\Ubuntu-20.04\root\git\olliOS\ollios.iso" "C:\temp\ollios.iso"
qemu-system-i386.exe -vga std -gdb tcp::1234 -drive file=C:\temp\ollios.iso,if=ide,index=0,media=cdrom