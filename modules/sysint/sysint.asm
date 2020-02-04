
extern sysint_handler_c;

global sysint_handler:
sysint_handler:;(void)

push ebp
push edi
push esi
push edx
push ecx
push ebx
push eax

call sysint_handler_c

pop eax
pop ebx
pop ecx
pop esi
pop edx
pop edi
pop ebp

iret
