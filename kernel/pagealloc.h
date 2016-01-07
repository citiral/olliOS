#ifndef __PAGE_ALLOC_H
#define __PAGE_ALLOC_H

#include "cdefs.h"
#include "types.h"
#include <stddef.h>

typedef u32 PageDirectory;
typedef u32 PageTable;

// a possible page directory entry that can be loaded into the cpu
// note that variables off this type will always be alligned on 4K boundaries.
typedef struct __PACKED __PAGE_ALIGNED PageEntry {
    PageTable pages[1024];
    u32 attributes;
    size_t entries;
} PageEntry;

// this initializes the page entry.
void initializeEntry(PageEntry& entry);


#endif /* end of include guard: __PAGE_ALLOC_H */
