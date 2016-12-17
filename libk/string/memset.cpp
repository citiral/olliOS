#include <string.h>
#include <stddef.h>

void* memset(void* ptr, int value, size_t num)
{
	unsigned char* data = (unsigned char*)ptr;
	
	for (size_t i = 0 ; i < num ; i++)
	{
		data[i] = (unsigned char)value;
	}

	return ptr;
}