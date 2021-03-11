
extern c_interrupt_wrapper
extern end_interrupt
extern printf

global isr_wrapper
align 4
isr_wrapper:
pushad
pushfd
cld
call c_interrupt_wrapper
popfd
popad
iret


extern intHandlerDummyKeyboard
global intHandlerDummyKeyboard_asm
align 4
intHandlerDummyKeyboard_asm:
cli
pushad
pushfd
cld
call intHandlerDummyKeyboard_asm
popfd
popad
sti
iret


extern intHandlerGeneralProtectionViolation
global intHandlerGeneralProtectionViolation_asm
align 4
intHandlerGeneralProtectionViolation_asm:
pushad
pushfd
cld
call intHandlerGeneralProtectionViolation
popfd
popad
iret


extern intHandlerPageFault
global intHandlerPageFault_asm
align 4
intHandlerPageFault_asm:
pushad
pushfd
cld
call intHandlerPageFault
popfd
popad
iret


