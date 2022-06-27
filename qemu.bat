D:
cd D:\Tools\qemu
dir
qemu-system-i386.exe -curses std -gdb tcp::1234 -drive file=root.iso,if=ide,index=0,media=cdrom