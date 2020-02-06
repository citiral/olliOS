
global sysint:
sysint:;(u32 eax, u32 ebx, u32 ecx, u32 edx, u32 esi, u32 edi, u32 ebp)

push eax
mov eax, [esp+8]

push ebx
mov ebx, [esp+16]

push ecx
mov ecx, [esp+24]

push edx
mov edx, [esp+32]

push esi
mov esi, [esp+40]

push edi
mov edi, [esp+48]

push ebp
mov ebp, [esp+56]

int 0x80 

pop ebp
pop edi
pop esi
pop edx
pop ecx
pop ebx
add esp,4

ret
