#ifndef __PAGING_H
#define __PAGING_H

#include "cdefs.h"
#include "types.h"
#include <stddef.h>

#define PFLAG_PRESENT 0x1
#define PFLAG_RW 0x2
#define PFLAG_USER 0x4
#define PFLAG_WRITETHROUGH 0x8
#define PFLAG_CACHEDISABLE 0x10
#define PFLAG_ACCESSED 0x20
#define PFLAG_DIRTY 0x40
#define PFLAG_LARGEPAGE 0x80
#define PFLAG_GLOBAL 0x100

#define PAGE_SIZE 0x1000

#define CR0_PAGING = 0x80000000
#define CR4_PSE = 0x00000010

// a single entry into the page directory
typedef struct PageDirectoryEntry {
	u32 value;

public:
	PageDirectoryEntry();

	void setAddress(void* address);
	// enables and disables flags of the entry
	void enableFlag(u32 flag);
	void disableFlag(u32 flag);
	bool getFlag(u32 flag);
} PageDirectoryEntry;

// a single entry into the page table
typedef struct PageTableEntry {
	u32 value;

public:
	PageTableEntry();

	void setAddress(void* address);
	// enables and disables flags of the entry
	void enableFlag(u32 flag);
	void disableFlag(u32 flag);
	bool getFlag(u32 flag);
} PageTableEntry;

// a page directory. This is the thing that the cr3 will point to to use these pages
typedef struct PageDirectory {
	// the entries in the directory
	PageDirectoryEntry entries[1024];
public:
	// clears the directory, setting everything to 0
	void clear();
	// inserts the pagetable at the given virtual address
	void insert(PageDirectoryEntry entry, void* vaddress);
	// gets the page directory entry at the given virtual address
	PageDirectoryEntry& get(void* vaddress);
	// uses the page directory
	void use();
} PageDirectory;

// a page table. instances of this struct should always be page aligned (0x1000)
typedef struct PageTable {
	// the entries in the table
	PageTableEntry entries[1024];
	// the amount of entries in the table
	u32 size;
public:
	// clears the table, setting everything to 0
	void clear();
	// maps all pages in the directory starting from the given page aligned address
	void mapAll(void* start);
	// adds an entry to the table
	bool push(PageTableEntry entry);
} PageTable;

void pagingEnablePaging();
void pagingDisablePaging();
void pagingEnablePSE();
void pagingDisablePSE();


#endif
