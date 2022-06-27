extern intHandlerAta1
extern intHandlerAta2

global intHandlerAta1_asm
intHandlerAta1_asm:
pushad
pushfd
cld
push 46
call intHandlerAta1
add esp, 4
popfd
popad
iret

global intHandlerAta2_asm
intHandlerAta2_asm:
pushad
pushfd
cld
push 47
call intHandlerAta2
add esp, 4
popfd
popad
iret