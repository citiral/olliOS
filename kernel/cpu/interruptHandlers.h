#ifndef __INTERRUPT_HANDLERS_H
#define __INTERRUPT_HANDLERS_H

#include "types.h"

void intHandlerWakeup(u32 interrupt);
void intHandlerUndefined(u32 interrupt);
void intHandlerGeneralProtectionViolation(u32 interrupt);
void intHandlerPageFault(u32 interrupt);
void intHandlerSpurious(u32 interrupt);

#endif /* end of include guard: __INTERRUPT_HANDLERS_H */
