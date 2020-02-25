#include "memory/virtual.h"
#include "memory/physical.h"
#include "linker.h"
#include "stdlib.h"

namespace memory {

	PageDirectory kernelPageDirectory __PAGE_ALIGNED;

	void initializePaging()
	{
		// We start by clearing the kernel page directory, although right now it should already be 0
		memset(&kernelPageDirectory, 0, sizeof(PageDirectory));

		// Then we can start allocating pages
		// The current whole first GB is mapped to the 4th GB, so we can start allocating pages from KERNEL_END_PHYSICAL (making sure we are rounded to 4kbs)
		char* currentFree = (char*)KERNEL_END_PHYSICAL;
		currentFree += 0x1000 - (((u32)currentFree) % 0x1000);
		
		// firstpage is the virtual address to reach the physical page we just allocated
		PageTable* firstPage = (PageTable*) (currentFree + 0xC0000000);
		
		// we reserve the memory in the physical manager so it isn't used again, and then clear the page
		physicalMemoryManager.reservePhysicalMemory(currentFree, 0x1000);
		memset(firstPage, 0, 0x1000);

		// the first MB is mapped to itself
		kernelPageDirectory.entries[0].setAddress(currentFree);
		kernelPageDirectory.entries[0].enableFlag(PFLAG_RW | PFLAG_PRESENT | PFLAG_OWNED);

		// we do this by giving it 4kb pages until we reach the first megabyte
		for (u32 i = 0 ; i < 1024 ; i += 1) {
			firstPage->entries[i].setAddress((void*)(i * 0x1000));
			firstPage->entries[i].enableFlag(PFLAG_RW | PFLAG_PRESENT | PFLAG_OWNED);
		}

		// Then we have to allocate kernel pagetables, one for each 4kb of the kernel	
		for (u32 pos = 0 ; pos < (u32)KERNEL_END_PHYSICAL ; pos += 0x400000) {
			int i = pos / 0x400000;
			// again we allocate continueing from currentfree
			currentFree += 0x1000;

			// and again page is an alias of the virtual address to the freshly allocated table
			PageTable* page = (PageTable*)(currentFree + 0xC0000000);

			// and again we reserve it and clear it
			physicalMemoryManager.reservePhysicalMemory(currentFree, 0x1000);
			memset(page, 0, 0x1000);

			// and putting it in the directory. since we want this to map to the 4th gigabyte, we start at 768 (0xC0000000)
			kernelPageDirectory.entries[768+i].setAddress(currentFree);
			kernelPageDirectory.entries[768+i].enableFlag(PFLAG_RW | PFLAG_PRESENT | PFLAG_OWNED);

			// then we have to fill in the pagetable itself
			for (u32 j = 0 ; j < 1024 ; j += 1) {
				page->entries[j].setAddress((void*)(i*0x400000 + j*0x1000));
				page->entries[j].enableFlag(PFLAG_RW | PFLAG_PRESENT | PFLAG_OWNED);
			}
		}

		// we set up one more page.. we bind the very last directory entry to the directory itself. This allows us to always write to the directory, since its physical memory might not be in our virtual address space
		kernelPageDirectory.entries[1023].setAddress(((char*)&kernelPageDirectory) - 0xC0000000);

		// we don't give him an OWNED flag, because we don't want to accidentally deallocate our directory.
		kernelPageDirectory.entries[1023].enableFlag(PFLAG_RW | PFLAG_PRESENT);

		// and finally we can use the directory
		((PageDirectory*)(((char*)&kernelPageDirectory) - 0xC0000000))->use();
		
		// we also make sure a directory is allocated for the kernel (last gb)
		for (int i = 256*3; i < 256*4; i++) {
			kernelPageDirectory.allocateEntry(i);
		}
	}

	void enablePaging()
	{
		asm volatile (
			"mov %%cr0, %%eax\n"
			"or $0x80000000, %%eax\n"
			"mov %%eax, %%cr0\n"
			:::"%eax");
	}

	void disablePaging()
	{
		asm volatile (
			"mov %%cr0, %%eax\n"
			"and $0x7FFFFFFF, %%eax\n"
			"mov %%eax, %%cr0\n"
			:::"%eax");
	}

	void enablePSE()
	{
		asm volatile (
			"mov %%cr4, %%eax\n"
			"or $0x00000010, %%eax\n"
			"mov %%eax, %%cr4\n"
			:::"%eax");
	}

	void disablePSE()
	{
		asm volatile (
			"mov %%cr4, %%eax\n"
			"and $0xFFFFFFEF, %%eax\n"
			"mov %%eax, %%cr4\n"
			:::"%eax");
	}

	PageTableItem::PageTableItem()
	{
		value = 0;
	}

	void PageTableItem::setAddress(void* address) {
		// mask out the old adress and add the new one
		value &= ~0xFFFFF000;
		value |= (u32)address;
	}

	void* PageTableItem::getAddress() {
		return (void*)(value & 0xFFFFF000);
	}

	void PageTableItem::enableFlag(u32 flag)
	{
		value |= flag;
	}

	void PageTableItem::disableFlag(u32 flag)
	{
		value &= 0xFFFFFFFF - flag;
	}

	bool PageTableItem::getFlag(u32 flag)
	{
		return (value & flag) == flag;
	}

	void PageDirectory::use()
	{
		// convert the entries from virtual to physical (kernel is virtually at 0xc0000000 but physically at 0x0)
		u32 pos = (u32)entries;

		// set the actualy control register determining which page is used
		asm volatile (
			"mov %%eax, %%cr3\n"
			:: "a" (pos)
		);

		// do an absolute jump to force the cpu to recognize the new directory
		asm volatile (
			"movl $1f, %%eax\n\
			jmp *%%eax\n\
			1: nop"
			::: "eax"
		);

		for (int i = 0 ; i < 1024 ; i++)
		invalidatePage((u32*)(0xFFC00000 + 0x1000*i));
	}

	void PageDirectory::forceUpdate() {
		// do an absolute jump to force the cpu to recognize the new directory
		asm volatile (
			"movl $1f, %%eax\n\
			jmp *%%eax\n\
			1: nop"
			::: "eax"
		);
	}

	PageDirectory* PageDirectory::current() {
		PageDirectory* page;

		// set the actualy control register determining which page is used
		asm volatile (
			"mov %%cr3, %0\n"
			: "=r" (page)
		);

		return page;
	}

	void PageDirectory::bindVirtualPage(void* page) {
		// we get the indexes related to that page
		int dirindex = ((u32)page) / 0x400000;
		int pageindex = (((u32)page) % 0x400000) / 0x1000;
		//printf("%d %d %x\n", dirindex, pageindex, getReadableEntryPointer(dirindex));
		// if the directory does not exist, allocate one
		if (!getReadableEntryPointer(dirindex)->getFlag(PFLAG_PRESENT)) {
			allocateEntry(dirindex);
		}

		// Only bind it if it isn't already mapped
		if (!getReadableTablePointer(dirindex, pageindex)->getFlag(PFLAG_PRESENT)) {
			// then we get us a physical page to use
			void* physpage = physicalMemoryManager.allocatePhysicalMemory();
			if (physpage == nullptr) {
				CPU::panic("failed allocating physical memory\n");
			}

			// put it in the page
			getReadableTablePointer(dirindex, pageindex)->setAddress(physpage);
			getReadableTablePointer(dirindex, pageindex)->enableFlag(PFLAG_RW | PFLAG_PRESENT | PFLAG_OWNED);

			// and then clear the memory
			memset(page, 0, 0x1000);
		}
	}

	// makes sure the given physical 4kb space is bound to some random virtual memory
	void* PageDirectory::bindPhysicalPage(void* physical, void* page) {	
		// we keep looping from this page until we find one that is free, or until we reach the end of the address space. We also don't want to allocate the last 4MB.
		page = (void*)((u32)page + ((u32)page % 0x1000 == 0 ? 0 : (0x1000 - ((u32)page % 0x1000))));
		int dirindex;
		do {
			// we get the indexes related to that page
			dirindex = ((u32)page) / 0x400000;
			int pageindex = (((u32)page) % 0x400000) / 0x1000;

			// if the entry doesnt exist, lets allocate one. Its content should be empty so the following checks should pass
			if (!getReadableEntryPointer(dirindex)->getFlag(PFLAG_PRESENT))
				allocateEntry(dirindex);

			// check if the table is used. if not, we allocate and return that one
			if (!getReadableTablePointer(dirindex, pageindex)->getFlag(PFLAG_PRESENT)) {
				mapMemory(page, physical);
				return page;
			}

			// and otherwise we advance by one page
			page = (void*)((u32)page + 0x1000);
		} while (dirindex < 1023);

		return nullptr;
	}

	void* PageDirectory::bindFirstFreeVirtualPage(void* page) {
		return bindFirstFreeVirtualPages(page, 1);
	}

	void* PageDirectory::bindFirstFreeVirtualPages(void* page, int count) {
		// first we round this up to the nearest page
		page = (void*)((u32)page + ((u32)page % 0x1000 == 0 ? 0 : (0x1000 - ((u32)page % 0x1000))));

		// this is the index of the start address
		int index = ((u32)page) / 0x1000;

		// we check from the index until the end of the memory, taking into acount the count of pages we want to allocate
		for (int i = index ; i < 1024*1024 - count + 1 ; i++) {
			bool spacefree = true;

			// then we check if count pages are free
			for (int c = i ; c < i + count ; c++) {
				int dirindex = i / 1024;
				int pageindex = i % 1024;
				if (!getReadableEntryPointer(dirindex)->getFlag(PFLAG_PRESENT))
					continue;

				if (!getReadableTablePointer(dirindex, pageindex)->getFlag(PFLAG_PRESENT))
					continue;

				spacefree = false;
				break;
			}

			// if the pages were free, we allocate them all
			if (spacefree) {
				for (int c = i ; c < i + count ; c++) {
					bindVirtualPage((void*)(c * 0x1000));
				}

				// and then we return the start
				return (void*)(i*0x1000);
			}
		}

		// if we find nothing, we return the nullptr
		return nullptr;
	}

	void PageDirectory::unbindVirtualPage(void* page) {
		// we get the indexes related to that page
		int dirindex = ((u32)page) / 0x400000;
		int pageindex = (((u32)page) % 0x400000) / 0x1000;

		// if there is no directory on this index, just move on
		if (!getReadableEntryPointer(dirindex)->getFlag(PFLAG_PRESENT))
			return;

		// and if the table is not present, move on
		if (!getReadableTablePointer(dirindex, pageindex)->getFlag(PFLAG_PRESENT))
			return;


		// deallocate the page if it is owned
		if (getReadableTablePointer(dirindex, pageindex)->getFlag(PFLAG_OWNED))
			physicalMemoryManager.freePhysicalMemory(getReadableTablePointer(dirindex, pageindex)->getAddress());

		// and the clear the entry
		getReadableTablePointer(dirindex, pageindex)->value = 0;

		// finally we invalidate the page so the CPU knows it's invalidate
		invalidatePage(page);
	}

	void PageDirectory::mapMemory(void* page, void* physical) {
		// we get the indexes related to that page
		int dirindex = (u32)page / 0x400000;
		int pageindex = ((u32)page % 0x400000) / 0x1000;

		// if the directory does not exist, allocate one
		if (!getReadableEntryPointer(dirindex)->getFlag(PFLAG_PRESENT))
			allocateEntry(dirindex);

		// put it in the page
		getReadableTablePointer(dirindex, pageindex)->setAddress(physical);
		getReadableTablePointer(dirindex, pageindex)->enableFlag(PFLAG_RW | PFLAG_PRESENT);
	}

	void PageDirectory::mapMemory(void* page, void* physical, size_t length) {
		// Convert the length from bytes to blocks of 4KiB
		if ((length & 0xFFF) == 0)
			length >>= 12;
		else
			length = (length >> 12) + 1;

		// Map them

		char* pg = static_cast<char*>(page);
		char* phys = static_cast<char*>(physical);
		for (size_t i = 0; i < length; i++) {
			mapMemory((void*) (pg + i*4096), (void*) (phys + i*4096));
		}
	}

	void PageDirectory::allocateEntry(int index) {
		// if the entry is already allocated, we don't have to do anything
		if (getReadableEntryPointer(index)->getFlag(PFLAG_PRESENT))
			return;

		// then we fetch some physical memory
		void* phys = physicalMemoryManager.allocatePhysicalMemory();
		if (phys == nullptr) {
			CPU::panic("failed allocating physical memory\n");
		}

		// put it in the entry
		getReadableEntryPointer(index)->setAddress(phys);
		
		// and set up the flags
		getReadableEntryPointer(index)->enableFlag(PFLAG_RW | PFLAG_PRESENT | PFLAG_OWNED);

		// now that it is bound we can clear the memory
		memset((((PageTableEntry*)0xFFC00000) + (index * 1024)), 0, 0x1000);
	}

	void PageDirectory::freeEntry(int index) {
		// if the entry doesn't exist, don't do anything
		if (!getReadableEntryPointer(index)->getFlag(PFLAG_PRESENT))
			return;

		// deallocate the memory if this directory is owned
		if (!getReadableEntryPointer(index)->getFlag(PFLAG_OWNED))
			return;

		// then we loop over all tables in the entry
		for (int i = 0 ; i < 1024 ; i++) {
			// if the page is owned, free it
			if (getReadableTablePointer(index, i)->getFlag(PFLAG_PRESENT)) {
				physicalMemoryManager.freePhysicalMemory(getReadableTablePointer(index, i)->getAddress());
			}
		}

		physicalMemoryManager.freePhysicalMemory(getReadableEntryPointer(index)->getAddress());
		
		// and then clear the entry
		getReadableEntryPointer(index)->value = 0;
	}

	PageDirectoryEntry* PageDirectory::getReadableEntryPointer(int index) {
		return ((PageDirectoryEntry*)0xFFFFF000) + index;
	}

	PageTableEntry* PageDirectory::getReadableTablePointer(int index, int tableindex) {
		return ((PageTableEntry*)0xFFC00000) + (index * 1024 + tableindex);
	}

	void PageDirectory::invalidatePage(int index, int tableindex) {
		invalidatePage((void*)(0x400000 * index + 0x1000 * tableindex));
	}

	void PageDirectory::invalidatePage(void* address) {
		asm volatile("invlpg (%0)" ::"r" (address) : "memory");
	}

	void* PageDirectory::getVirtualAddress(void* physical) {
		// Loop over page directories
		void* phys = (void*) ((size_t)(physical) & ~((size_t)4095));
		for (int pdi = 0; pdi < 1024; pdi++) {
			PageDirectoryEntry* entry = getReadableEntryPointer(pdi);
			if (entry->getAddress() != 0)
			{
				for (int pti = 0; pti < 1024; pti++) {
					PageTableEntry* entry = getReadableTablePointer(pdi, pti);
					if (entry->getAddress() == phys) {
						return (void*) ((((size_t)pdi) * 4096*1024) + (((size_t)pti) * 4096) + ((size_t) physical&4095));
					}
				}
			}
		}
		return 0;
	}

	void* PageDirectory::getPhysicalAddress(void* virt) {
		int pdIndex = ((size_t)(virt) >> 12) >> 10;
		int ptIndex = ((size_t)(virt) >> 12) & (1024-1);
		PageTableEntry* page = getReadableTablePointer(pdIndex, ptIndex);
		return (void*) ((size_t) page->getAddress() + ((size_t) virt & (size_t)(4095)));
	}

	PageDirectory* PageDirectory::clone() {
		PageDirectory* dir = memory::allocatePageDirectory();

		memcpy(dir->entries, this->entries, sizeof(this->entries));

		dir->entries[1023].setAddress(kernelPageDirectory.getPhysicalAddress(dir));
		dir->entries[1023].enableFlag(PFLAG_RW | PFLAG_PRESENT);

		for (int i = 0 ; i < 1024 ; i++) {
			dir->entries[i].disableFlag(PFLAG_OWNED);
		}

		return dir;
	}

	PageDirectory* PageDirectory::deep_clone() {
		PageDirectory* dir = clone();

		u8* buffer = (u8*) malloc(0x1000);
		memset(buffer, 0, sizeof(buffer));

		((PageDirectory*)getPhysicalAddress(dir))->use();
		void* phys;
		for (int i = 1 ; i < 256*3 ; i++) {
			if (getReadableEntryPointer(i)->getFlag(PFLAG_PRESENT)) {

				// Copy the old page
				memcpy(buffer, (((PageTableEntry*)0xFFC00000) + (i * 1024)), 0x1000);

				// Allocate a new physical page
				void* phys = physicalMemoryManager.allocatePhysicalMemory();
				if (phys == nullptr) {
					CPU::panic("failed allocating physical memory\n");
				}

				// put it in the entry
				dir->getReadableEntryPointer(i)->setAddress(phys);
				dir->getReadableEntryPointer(i)->enableFlag(PFLAG_OWNED | PFLAG_PRESENT);
				invalidatePage((u8*)(0xFFC00000 + i * 1024 * sizeof(void*)));
				
				// And copy the buffer back to the new physical page
				memcpy((((PageTableEntry*)0xFFC00000) + (i * 1024)), buffer, 0x1000);

				// Now copy all pages of the directory
				for (int k = 0 ; k < 1024 ; k++) {
					if (dir->getReadableTablePointer(i, k)->getFlag(PFLAG_PRESENT)) {
						// Copy the old page
						memcpy(buffer, (void*)(i * 0x400000 + k * 0x1000), 0x1000);

						// Allocate a new physical page
						phys = physicalMemoryManager.allocatePhysicalMemory();
						if (phys == nullptr) {
							CPU::panic("failed allocating physical memory\n");
						}

						// Put it in the entry
						getReadableTablePointer(i, k)->setAddress(phys);
						dir->getReadableTablePointer(i, k)->enableFlag(PFLAG_RW | PFLAG_PRESENT);
						invalidatePage((void*)(i * 0x400000 + k * 0x1000));
						
						// And copy the contents back
						memcpy((void*)(i * 0x400000 + k * 0x1000), buffer, 0x1000);
					}
				}
			}
		}

		((PageDirectory*)dir->getPhysicalAddress(this))->use();
		
		free(buffer);

		return dir;
	}

	PageDirectory* allocatePageDirectory() {
		// get a virtual page to use, these are by definition page aligned
		PageDirectory* page = (PageDirectory*)kernelPageDirectory.bindFirstFreeVirtualPage(KERNEL_END_VIRTUAL);

		// clear it
		memset(page, 0, sizeof(PageDirectory));

		return page;
	}

	void freePageDirectory(PageDirectory* page) {
		bool eflag = CLI();

		PageDirectory* current = PageDirectory::current();

		// First we bind the page directory so we can read all of it
		((PageDirectory*)current->getPhysicalAddress(page))->use();

		// Loop over all entries in the pagetable up to 0xC0000000
		// We never free the kernel space in this way
		for (size_t i = 1 ; i < 256*3 ; i++) {
			page->freeEntry(i);
		}

		((PageDirectory*)page->getPhysicalAddress(current))->use();
		current->unbindVirtualPage(page);

		STI(eflag);
	}
}