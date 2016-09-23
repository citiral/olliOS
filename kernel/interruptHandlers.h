#ifndef __INTERRUPT_HANDLERS_H
#define __INTERRUPT_HANDLERS_H

#include "types.h"

void intHandlerUndefined(u32 interrupt);
void intHandlerKeyboard(u32 interrupt);
#endif /* end of include guard: __INTERRUPT_HANDLERS_H */