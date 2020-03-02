#ifndef __VIRTUAL_H
#define __VIRTUAL_H

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

namespace memory {

	class PageTableItem {
	public:
		PageTableItem();

		// gets and sets the address of the item, properly masked
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
		// the entries in the table
		PageTableEntry entries[1024];
	};

	// a page directory. This is the thing that the cr3 will point to to use these pages
	class PageDirectory {
	public:
		// uses the page directory
		void use();

		// forces reloading the pagetable by a longjump. This just forces the update, an update can be triggered automatically regardless of this function
		void forceUpdate();

		// makes sure the given virtual 4kb space is bound to some random physical memory
		void bindVirtualPage(void* page);
		
		// makes sure the given physical 4kb space is bound to some random virtual memory, starting at virtual offset (default 0)
		void* bindPhysicalPage(void* physical, void* start = 0);

		// binds the first free virtual 4kb page starting from the given virtual address
		void* bindFirstFreeVirtualPage(void* page);

		// binds the first free count consecutive virtual 4kb pages starting from the given virtual address
		void* bindFirstFreeVirtualPages(void* page, int count);

		// releases the given virtual 4kb space, deallocating the physical memory if it was allocated by bindVirtualPage or bindFirstFreeVirtualPage
		void unbindVirtualPage(void* page);

		// maps the given 4kb space to the given physical address
		void mapMemory(void* page, void* physical);
		
		// Maps the given length-sized to the given physical address
		// Note that it can only map 4kb spaces internally so a bit more might get allocated
		void mapMemory(void* page, void* physical, size_t length);

		// Gets the current page directory
		static PageDirectory* current();

		// sets up the entry at the given address by allocating physical memory and setting up the right flags
		void allocateEntry(int index);

		// releases the entry at the given address, releasing its physical memory. This frees all memory referenced by this entry allocated by bindVirtualPage or bindFirstFreeVirtualPage
		void freeEntry(int index);

		// returns a virtual pointer to the entry at the index that can be referenced
		PageDirectoryEntry* getReadableEntryPointer(int index);

		// returns a virtual pointer to a table in the entry at the index that can be referenced
		PageTableEntry* getReadableTablePointer(int index, int tableindex);

		// uses teh INVLPG to invalidate a cached TLB entry. This makes sure that page changes are recognized.
		void invalidatePage(int index, int tableindex);
		void invalidatePage(void* address);

		// Get the address in virtual space for a given physical address
		void* getVirtualAddress(void* physical);

		// Get the address in physical space for a given virtual address
		void* getPhysicalAddress(void* virt);

		// (shallow) clones the current pagetable, the entries of the clone will point to the same directories as the original
		PageDirectory* clone();
		
		// (deeps) clones the current pagetable, the entries of the clone (except the ones in kernelspace) will point to the new directies with copies of the physical memory
		PageDirectory* deep_clone();

		// the entries in the directory
		PageDirectoryEntry entries[1024];
	};

	void initializePaging();
	void enablePaging();
	void disablePaging();
	void enablePSE();
	void disablePSE();

	PageDirectory* allocatePageDirectory();
	void freePageDirectory(PageDirectory* page);

	// the pagetable used in the kernel. Because we just memory map the last gigabyte a 4mb page directory is used
	extern PageDirectory kernelPageDirectory __PAGE_ALIGNED;
}

#endif