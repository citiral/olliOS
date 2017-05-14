#include "memory/virtual.h"
#include "memory/physical.h"
#include "linker.h"

/*	mov %cr4, %eax
	or $0x00000010, %eax
	mov %eax, %cr4

	# enable paging
	mov %cr0, %eax
	or $0x80000000, %eax
	mov %eax, %cr0*/

PageDirectory kernelPageDirectory __PAGE_ALIGNED;

void PageInit()
{
	// We start by clearing the kernel page directory, although right now it should already be 0
	memset(&kernelPageDirectory, 0, sizeof(PageDirectory));

	// Then we can start allocating pages
	// The current whole first GB is mapped to the 4th GB, so we can start allocating pages from KERNEL_END_PHYSICAL (making sure we are rounded to 4kbs)
	char* currentFree = (char*)KERNEL_END_PHYSICAL;
	currentFree += 0x1000 - (((u32)currentFree) % 0x1000);
	printf("currentfree: %X", (u32)currentFree);
	
	// firstpage is the virtual address to reach the physical page we just allocated
	PageTable* firstPage = (PageTable*) (currentFree + 0xC0000000);
	
	// we reserve the memory in the physical manager so it isn't used again, and then clear the page
	physicalMemoryManager.reservePhysicalMemory(currentFree, 0x1000);
	memset(firstPage, 0, 0x1000);

	// the first MB is mapped to itself
	kernelPageDirectory.entries[0].setAddress(currentFree);
	kernelPageDirectory.entries[0].enableFlag(PFLAG_RW | PFLAG_PRESENT);

	// we do this by giving it 4kb pages until we reach the first megabyte
	for (u32 i = 0 ; i < 1024 ; i += 1) {
		firstPage->entries[i].setAddress((void*)(i * 0x1000));
		firstPage->entries[i].enableFlag(PFLAG_RW | PFLAG_PRESENT);
	}

	// Then we have to allocate kernel pagetables, one for each 4MB of the kernel	
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
		kernelPageDirectory.entries[768+i].enableFlag(PFLAG_RW | PFLAG_PRESENT);

		// then we have to fill in the pagetable itself
		for (u32 j = 0 ; j < 1024 ; j += 1) {
			page->entries[j].setAddress((void*)(i*0x400000 + j*0x1000));
			page->entries[j].enableFlag(PFLAG_RW | PFLAG_PRESENT);
		}
		printf("made page");

	}
    BOCHS_BREAKPOINT

	// Then finally, we can use the kernel page
	// set the actualy control register determining which page is used
	asm volatile (
		"mov %%eax, %%cr3\n"
		:: "a" ((u32)&kernelPageDirectory - 0xC0000000)
	);


    // do an absolute jump to force the cpu to recognize the new directory
    asm volatile (
        "movl $1f, %%eax\n\
        jmp *%%eax\n\
        1: nop"
        ::: "eax"
    );
}

void pagingEnablePaging()
{
	asm volatile (
		"mov %%cr0, %%eax\n"
		"or $0x80000000, %%eax\n"
		"mov %%eax, %%cr0\n"
		:::"%eax");
}

void pagingDisablePaging()
{
	asm volatile (
		"mov %%cr0, %%eax\n"
		"and $0x7FFFFFFF, %%eax\n"
		"mov %%eax, %%cr0\n"
		:::"%eax");
}

void pagingEnablePSE()
{
	asm volatile (
		"mov %%cr4, %%eax\n"
		"or $0x00000010, %%eax\n"
		"mov %%eax, %%cr4\n"
		:::"%eax");
}

void pagingDisablePSE()
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

void PageTableItem::allocate() {
	setAddress(PageTable::create());
	enableFlag(PFLAG_PRESENT);
}

void PageTableItem::deallocate() {
	physicalMemoryManager.freePhysicalMemory(getAddress());
	disableFlag(PFLAG_PRESENT);
}

void PageTableItem::setAddress(void* address) {
	// mask out the old adress and add the new one
	value &= 0xFFFFF000;
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

void PageDirectory::set(PageDirectoryEntry entry, int index)
{
	entries[index] = entry;
}

PageDirectoryEntry& PageDirectory::get(int index)
{
	//size_t index = (size_t)vaddress / 0x1000;
	return entries[index];
}

void PageDirectory::use()
{
    // convert the entries from virtual to physical (kernel is virtually at 0xc0000000 but physically at 0x0)
	u32 pos = (u32)&entries - 0xC0000000;

    // set the actualy control register determining which page is used
	asm volatile (
		"mov %%eax, %%cr3\n"
		:: "a" (pos)
	);

    BOCHS_BREAKPOINT

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

PageDirectory* PageDirectory::create() {
	PageDirectory* directory = (PageDirectory*)physicalMemoryManager.allocatePhysicalMemory();

	// then we map the pointer to the directory into the top of its own adress space
	/*directory->entries[1023].allocate();
	directory->entries[1023].setFlag(PFLAG_OWNED);
	directory->entries[1023].getAddress().entries[1023].setAddress(directory);
	directory->entries[1023].getAddress().entries[1023].setFlag(PFLAG_PRESENT);*/
	
	return directory;
}

void PageDirectory::destroy() {
	physicalMemoryManager.freePhysicalMemory(this);

	/*for (int i = 0 ; i < 1024 ; i++) {
		if (entries[i].getFlag(PFLAG_OWNED)) {
			entries[i].getAddress().destroy();
			entries[i].deallocate();
		}
	}*/
}

PageTable* PageTable::create() {
	PageTable* table = (PageTable*)physicalMemoryManager.allocatePhysicalMemory();
	return table;
}

void PageTable::destroy() {
	physicalMemoryManager.freePhysicalMemory(this);
}

void PageTable::set(PageTableEntry entry, int index)
{
    entries[index] = entry;
}

PageTableEntry& PageTable::get(int index)
{
    return entries[index];
}

void PageTable::mapAll(void* start)
{
	for (size_t i = 0 ; i < 1024 ; i++)
	{
		entries[i].value = 0;
		entries[i].setAddress((char*)start + (i * 0x1000));
		entries[i].enableFlag(PFLAG_PRESENT | PFLAG_RW);
	}
}