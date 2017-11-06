#ifndef __INTERRUPT_HANDLERS_H
#define __INTERRUPT_HANDLERS_H

#include "types.h"

// Normal undefined handler
void intHandlerUndefined(u32 interrupt);
//void intHandlerKeyboard(u32 interrupt);
void intHandlerAta(u32 interrupt);
void intHandlerGeneralProtectionViolation(u32 interrupt);
void intHandlerPageFault(u32 interrupt);
void intHandlerSpurious(u32 interrupt);
#endif /* end of include guard: __INTERRUPT_HANDLERS_H */
