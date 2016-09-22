#include "paging.h"


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
    kernelPageDirectory.clear();

    // identity map the first four megabytes
    kernelPageDirectory.get(0).setAddress((void*)0x00000000);
    kernelPageDirectory.get(0).enableFlag(PFLAG_PRESENT | PFLAG_RW | PFLAG_LARGEPAGE);

    // map the last gb to the first gb
    for (int i = 0 ; i < 0x40000000 / 0x00400000; i++) {
        kernelPageDirectory.get(i + 768).setAddress((void*)0x00000000 + 0x00400000*i);
        kernelPageDirectory.get(i + 768).enableFlag(PFLAG_PRESENT | PFLAG_RW | PFLAG_LARGEPAGE);
    }

    kernelPageDirectory.use();
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

PageDirectoryEntry::PageDirectoryEntry()
{
	value = 0;
}

void PageDirectoryEntry::setAddress(void* address)
{
	// mask out the old adress and add the new one
	value &= 0x00000FFF;
	value |= (u32)address;
}

void PageDirectoryEntry::enableFlag(u32 flag)
{
	value |= flag;
}

void PageDirectoryEntry::disableFlag(u32 flag)
{
	value &= 0xFFFFFFFF - flag;
}

bool PageDirectoryEntry::getFlag(u32 flag)
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

PageTableEntry::PageTableEntry()
{
	value = 0;
}

void PageTableEntry::setAddress(void* address)
{
	// mask out the old adress and add the new one
	value &= 0x00000FFF;
	value |= (u32)address;
}

void PageTableEntry::enableFlag(u32 flag)
{
	value |= flag;
}

void PageTableEntry::disableFlag(u32 flag)
{
	value &= 0xFFFFFFFF - flag;
}

bool PageTableEntry::getFlag(u32 flag)
{
	return (value & flag) == flag;
}

void PageDirectory::clear()
{
	for (size_t i = 0 ; i < 1024 ; i++)
	{
		entries[i].value = 0;
	}
}

void PageTable::clear()
{
	for (size_t i = 0 ; i < 1024 ; i++)
	{
		entries[i].value = 0;
	}
}


void PageTable::set(PageTableEntry entry, int index)
{
    entries[index] = entry;
}

PageTableEntry PageTable::get(int index)
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
