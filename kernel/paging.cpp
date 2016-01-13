#include "paging.h"


/*	mov %cr4, %eax
	or $0x00000010, %eax
	mov %eax, %cr4

	# enable paging
	mov %cr0, %eax
	or $0x80000000, %eax
	mov %eax, %cr0*/

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

void PageDirectory::insert(PageDirectoryEntry entry, void* vaddress)
{
	size_t index = (size_t)vaddress / 0x1000;
	entries[index] = entry;
}

PageDirectoryEntry& PageDirectory::get(void* vaddress)
{
	//size_t index = (size_t)vaddress / 0x1000;
	size_t index = (size_t)vaddress / 0x1000;
	return entries[index];
}

void PageDirectory::use()
{
	u32 pos = (u32)entries - 0xC0000000;

	asm volatile (
		"mov %0, %%eax\n"
		"mov %%eax, %%cr3\n"
		:: "r" (pos)
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
	size = 0;
}

bool PageTable::push(PageTableEntry entry)
{
	if (size < 1024)
	{
		entries[size] = entry;
		size++;
	}
}

void PageTable::mapAll(void* start)
{
	for (size_t i = 0 ; i < 1024 ; i++)
	{
		entries[i].value = 0;
		entries[i].setAddress((char*)start + (i * 0x1000));
		entries[i].enableFlag(PFLAG_PRESENT | PFLAG_RW);
	}
	size = 1024;
}
