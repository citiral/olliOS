;we declare the multiboot header so the bootloader can find this and load the kernel
MBALIGN		equ		1<<0			;align loaded modules on page boundaries
MEMINFO		equ		1<<1			;provide memory map
FLAGS		equ 	MBALIGN | MEMINFO 	;we generaet the actual flags
MAGIC		equ 	0x1BADB002			;magic number so the bootloader can find the header
CHECKSUM	equ 	-(MAGIC + FLAGS)	;checksum for the above flag, so we can really prove we are multiboot




;we put the multiboot data in a seperate section so the linker can put this on the top
;of the file, so the bootloader can find this section
section .multiboot
align 4
	dd	MAGIC
	dd	FLAGS
	dd	CHECKSUM

;make a section for the stack
section .bootstrap_stack, nobits
align 4
stack_bottom:
times 16384 db 0 ; this is 16KB of stack
stack_top:


;the linker script will define _start to be the entry point of the kernel
;so the bootloader will jump to this section

section .text
global _start:function (_start.end - _start)

_start: 
	;first we set up our stack register to the stack we previously allocated
	;the stack grows downwards so we move the top value of the stack
	mov esp, stack_top

	;now we jump into the actual entry point
	extern main
	call main

	;if this ever returns we let the pc hang
	;first we disable interrupts
	cli
	;and then we halt so it does nothing
	hlt

	;if it ever continues, we hardwait anyway
.hang:
	jmp .hang

.end:


section .data
db 0 
section .bss
db 0