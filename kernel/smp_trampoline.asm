; We define a smp_trampoline_section that will be located in the first MB of memory. This because it will be executed in real mode

;extern GDTPtr

;align 0x1000
; The first thing this does is switch to protected mode, and switch to a normal text section of code to continue initialisation
;global smp_trampoline_entry

BITS 16
global smp_trampoline_entry
smp_trampoline_entry:
mov ax,0xDEAD
mov bx,0xBEEF
hlt
;jmp smp_trampoline_entry
;cli            ; disable interrupts
;lgdt [GDTPtr]    ; load GDT register with start address of Global Descriptor Table
;mov eax, cr0
;or al, 1       ; set PE (Protection Enable) bit in CR0 (Control Register 0)
;mov cr0, eax
;jmp 08h:smp_trampoline_init

;smp_trampoline_init:
;jmp smp_trampoline_init