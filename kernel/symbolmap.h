#ifndef __SYMBOLMAP_H
#define __SYMBOLMAP_H

#include <types.h>
#include "kstd/string.h"
#include "kstd/unordered_map.h"

struct SymbolMapEntry {
    u32 offset;
    char type;
};

class SymbolMap {
public:
    SymbolMap(const char* data);

    SymbolMapEntry* find_symbol(const char* name);

private:
    std::unordered_map<std::string, SymbolMapEntry> symbols;
};

#endif
