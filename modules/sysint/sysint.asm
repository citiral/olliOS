
extern sysint_handler_c;

global sysint_handler:
sysint_handler:;(void)

cld

push ebp
push edi
push esi
push edx
push ecx
push ebx
push eax

call sysint_handler_c

add esp,28

iret
