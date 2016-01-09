#include "pagealloc.h"
#include "stdio.h"
#include "string.h"
#include "cdefs.h"

#define MAX_T0 32
#define BITFIELD_SIZE 32768 // = 2^MAX_T0 / (0x1000 * 32)

// we keep two ways of keeping page information: through a bitmap
u32 bitfield[BITFIELD_SIZE];
// and through a stack
void* freestack[10000];
// amount of available on the stack
u32 freestacksize = 0;
// start index of clean pages, that haven't been used, and thus are not on the stack but are free.
u32 cleanindex = 0;

u32 T0;

bool getBitfield(u32 index)
{
	return (bitfield[index / 32] >> (31 - (index % 32))) & 1;
}

void setBitfield(u32 index)
{
	bitfield[index / 32] |= 1 << (31 - (index % 32));
}

void unsetBitfield(u32 index)
{
	bitfield[index / 32] &= 0 << (31 - (index % 32));
}

// traverses the bitfield as it represents the memory of the buddy allocator and returns the first free memory
// returns BITFIELD_SIZE + 1 if there is no available address
size_t traverseBitfield(char* start, char* stop, size_t requestSize, size_t curSize)
{
	// get the index of the center page
	u32 center = (stop - start)/0x1000;

	// if this index is free
	if (getBitfield(center) == 0)
	{
		//if this is the smallest size available, return the address
		if (requestSize > (curSize/2))
		{
			return center;
		}
		//otherwise, check the two neighborsf
	}
	return 0;
}

u32 getOrder(size_t length)
{
	u32 T = 0;
	while (length > 1) {
		length /= 2;
		T++;
	}
	return T;
}

u32 pow2(u32 n)
{
	u32 r = 1;
	while (n >= 1) {
		r *= 2;
		n--;
	}
	return r;
}

void pageStackPush(void* fyslocation)
{
	freestack[freestacksize] = fyslocation;
	freestacksize++;
}

void* pageStackPop()
{
	return freestack[--freestacksize];
}

void pageAllocatorInitialize(void* start, size_t length)
{
	// set the start, rounded to 4k blocks
	start = (char*)start + 0x1000 - ((u32)start % 0x1000);

	// round the length to page boundaries
	length = length - ((u32)length % 0x1000);

	// we then push all pages on the stack
	//for (size_t i = 0 ; i < length ; i += 0x1000)
	//{
	//	pageStackPush((char*)start + i);
	//}
}

void pagePrintStatus()
{
	printf("free pages on stack: %d\n", freestacksize);
}

// allocates a single page and returns it
PageTableEntry pageAlloc()
{
	//get a free location
	void* location = pageStackPop();
	PageTableEntry entry;
	entry.setAddress(location);

	return entry;
}
