#ifndef __PAGING_H
#define __PAGING_H

#include "cdefs.h"
#include "types.h"
#include <stddef.h>

#define PFLAG_PRESENT 0x1
#define PFLAG_RW 0x2
#define PFLAG_USER 0x4
#define PFLAG_WRITETHROUGH 0x8
#define PFLAG_CACHEDISABLE 0x16
#define PFLAG_ACCESSED 0x32
#define PFLAG_DIRTY 0x64
#define PFLAG_LARGEPAGE 0x128
#define PFLAG_GLOBAL 0x256

#define CR0_PAGING = 0x80000000
#define CR4_PSE = 0x00000010

//a single entry into the page directory
typedef struct __PACKED PageDirectoryEntry {
	u32 value;

public:
	PageDirectoryEntry(void* address);

	// enables and disables flags of the entry
	void enableFlag(u32 flag, bool value);
	void disableFlag(u32 flag, bool value);
	bool getFlag(u32 flag);
} PageDirectoryEntry;

//a single entry into the page table
typedef struct __PACKED PageTableEntry {
	u32 value;

public:
	PageTableEntry(void* address);

	// enables and disables flags of the entry
	void enableFlag(u32 flag, bool value);
	void disableFlag(u32 flag, bool value);
	bool getFlag(u32 flag);
} PageTableEntry;

void pagingEnablePaging();
void pagingDisablePaging();
void pagingEnablePSE();
void pagingDisablePSE();
void pagingUseTable(void* table);


#endif