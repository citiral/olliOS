; We define a smp_trampoline_section that will be located in the first MB of memory.

BITS 16

section .data.realmode
; a pointer to a stack to be used by this core. We need this asap so we can jump to c code
global smp_stack
smp_stack       dd  1
; a pointer to the kernel pagetable
global smp_page
smp_page        dd  1
; the C++ entry point which the trampoline will enter
global smp_entry_point
smp_entry_point dd  1

gdtptr:
; the gdt values to be used during the trampoline
global smp_gdt_size
smp_gdt_size    dw  1
global smp_gdt_offset
smp_gdt_offset  dd  1

section .text.realmode
global smp_trampoline_entry
smp_trampoline_entry:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; First we move out of real mode to protected mode ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; disable nmi
in al, 0x70
or al, 0x80
out 0x70, al

; disable other interrupts
cli

; enable the a20 line
in al, 0x92
or al, 2
out 0x92, al

; load the gdt
lgdt [gdtptr]

; enable protected mode
mov eax, cr0
or eax, 1
mov cr0, eax

; and jump to the 32 bit section of our trampoline
jmp 08h:smp_trampoline_32

BITS 32
smp_trampoline_32:
; load the segment registers
mov ax, 0x10
mov	DS, ax
mov	ES, ax
mov	FS, ax
mov	GS, ax
mov	SS, ax

; load the pagetable
mov eax, [smp_page]
mov cr3, eax

; enable paging
mov eax, cr0
or eax, 0x80000000
mov cr0, eax

; load the stack
mov esp, [smp_stack]

; enter our C++ entry point
mov eax, 0xdeadbeef
extern SmpEntryPoint
call SmpEntryPoint

; this could should never be reached
smp_end:
cli
hlt
jmp smp_end

global end_smp_trampoline_entry
end_smp_trampoline_entry: