#ifndef __ELF_H
#define __ELF_H

#include "types.h"
#include "symbolmap.h"

namespace elf {

void dump_elf(u8* file, SymbolMap& map);

}

#endif
