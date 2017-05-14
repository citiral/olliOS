#pragma once

#include "cdefs.h"
#include "types.h"
#include <stddef.h>

// set if the contents of the page is currently located in physical memory. If this is false when the page is loaded, a pagefault will be thrown.
#define PFLAG_PRESENT 0x1
#define PFLAG_RW 0x2
#define PFLAG_USER 0x4
#define PFLAG_WRITETHROUGH 0x8
#define PFLAG_CACHEDISABLE 0x10
#define PFLAG_ACCESSED 0x20
#define PFLAG_DIRTY 0x40
// if set the page is 4MB, otherwise it is 4kb
#define PFLAG_LARGEPAGE 0x80
#define PFLAG_GLOBAL 0x100
// if this flag is set, the entry will be deallocated by the pagedirectory on destruction
#define PFLAG_OWNED 0x200

#define PAGE_SIZE 0x1000

#define CR0_PAGING = 0x80000000
#define CR4_PSE = 0x00000010

class PageTableItem {
public:
	PageTableItem();

    // Allocates a page for the entry
    void allocate();

	// Deallocates a page for the entry
    void deallocate();

	void setAddress(void* address);
	void* getAddress();

	// enables and disables flags of the entry
	void enableFlag(u32 flag);
	void disableFlag(u32 flag);
	bool getFlag(u32 flag);

    u32 value;
};

// a single entry into the page table
class PageTableEntry : public PageTableItem {};

// a single entry into the page directory
class PageDirectoryEntry : public PageTableItem {};

// a page table. instances of this struct should always be page aligned (0x1000)
class PageTable {
public:
	// Allocates a new page table, on a page aligned address
	static PageTable* create();

	// Destroys this page table
	void destroy();

	// maps all pages in the directory starting from the given page aligned address
	void mapAll(void* start);

	// adds an entry to the table
	void set(PageTableEntry entry, int index);

    // gets an entry from the table
    PageTableEntry& get(int index);
	
    // the entries in the table
    PageTableEntry entries[1024];
};

// a page directory. This is the thing that the cr3 will point to to use these pages
class PageDirectory {
public:
	// Allocates a new page directory, on a page aligned address
	static PageDirectory* create();

	// Destroys this page directory
	void destroy();

	// sets the pagetable at the given index
	void set(PageDirectoryEntry entry, int index);

	// gets the page directory entry at the given virtual address
	PageDirectoryEntry& get(int index);

	// uses the page directory
	void use();
	static PageDirectory* current();

    // the entries in the directory
    PageDirectoryEntry entries[1024];
};

void PageInit();
void pagingEnablePaging();
void pagingDisablePaging();
void pagingEnablePSE();
void pagingDisablePSE();

// the pagetable used in the kernel. Because we just memory map the last gigabyte a 4mb page directory is used
extern PageDirectory kernelPageDirectory __PAGE_ALIGNED;
