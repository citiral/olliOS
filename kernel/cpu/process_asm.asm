extern printf

teststr:
db "test %X\n", 0

testfunc:
push 0x20
push teststr
call printf
add esp, 8
ret


global jump_usermode

;jump_usermode(func, arg1, arg2, arg3)
jump_usermode:

; get the function pointer argument into ecx (save because ecx is caller saved)
mov ecx, [esp+4]

; change all needed segment registers to that the GDT entry of ringlevel 3 data (with last 2 bits high to indicate R3)
mov ax, 0x23
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax

push 0
push 0
push 0

; save the current stack pointer in esp
mov eax, esp

; push user data segment to the GDT entry of ringlevel 3 data (with last 2 bits high to indicate R3)
push 0x23

; push stackpointer to use for the iret
push eax

; push eflags to use for iret
pushf

; push user code segment to the GDT entry of ringlevel 3 data (with last 2 bits high to indicate R3)
push 0x1B

; and push the function to jump to as ringlevel 3, which was our argument
push ecx

; The iret will pop all this from the stack, and jump to our function and setting eflags to ringlevel 3
iret

loop:
jmp loop