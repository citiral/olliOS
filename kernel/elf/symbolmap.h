#ifndef __SYMBOLMAP_H
#define __SYMBOLMAP_H

#include <types.h>
#include "kstd/string.h"
#include "kstd/vector.h"
#include "kstd/unordered_map.h"

struct SymbolMapEntry {
    u32 offset;
    char type;
    std::string name;
};

class SymbolMap {
public:
    SymbolMap(const char* data);

    SymbolMapEntry* find_symbol(const char* name);

    SymbolMapEntry* find_function_name(u32 addr);

private:
    std::unordered_map<std::string, SymbolMapEntry> symbols_map;
    std::vector<SymbolMapEntry> symbols_list;
};

extern SymbolMap* symbolMap;

#endif
