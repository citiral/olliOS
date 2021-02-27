extern sysint_handler_c;

global sysint_handler:
sysint_handler:;(void)

pushfd
push ds

push ebp
push edi
push esi
push edx
push ecx
push ebx
push eax

mov ax, 0x10
mov ds, ax

call sysint_handler_c

add esp,20

pop edx
pop ecx

pop ds
popfd

sysexit
