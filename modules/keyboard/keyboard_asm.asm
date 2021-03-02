extern intHandlerKeyboard

extern intHandlerKeyboard_asm
intHandlerKeyboard_asm:
pushad
pushfd
cld
push 33
call intHandlerKeyboard
add esp, 4
popfd
popad
iret