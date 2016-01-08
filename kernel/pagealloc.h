#ifndef __PAGE_ALLOC_H
#define __PAGE_ALLOC_H

#include "cdefs.h"
#include "types.h"
#include <stddef.h>
#include "paging.h"

/**typedef struct __PACKED __PAGE_ALIGNED PageDirectoryEntry {
    PageTableEntry pages[1024];
    u32 attributes;
    size_t entries;

public:
	PageDirectoryEntry();

	// enables and disables flags of the directory
	void enableFlag(u32 flag, bool value);
	void disableFlag(u32 flag, bool value);
	bool getFlag(u32 flag);
} PageEntry;**/

 
// this initializes the page entry.
void pageAllocatorInitialize(void* start, size_t length);
void pagePrintStatus();

// allocates a single page and returns it
PageTableEntry pageAlloc();




#endif /* end of include guard: __PAGE_ALLOC_H */
