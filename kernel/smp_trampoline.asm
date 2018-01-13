; We define a smp_trampoline_section that will be located in the first MB of memory.

BITS 16

section .data.realmode
; a pointer to a stack to be used by this core. We need this asap so we can jump to c code
smp_stack       resd    1
; and a pointer to the kernel pagetable
smp_page        resd    1

gdtptr:
; the gdt values to be used during the trampoline
global smp_gdt_size
smp_gdt_size    resw    1
global smp_gdt_offset
smp_gdt_offset  resd    1

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
or al, 1
mov cr0, eax

; and jump to the 32 bit section of our trampoline
jmp 08h:smp_trampoline_32


BITS 32
smp_trampoline_32:
; then we reenable nmi
in al, 0x70
and al, 0x7F
out 0x70, ax

mov eax, 0xDEADBEEF
hlt

global end_smp_trampoline_entry
end_smp_trampoline_entry: