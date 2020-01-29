#include <string.h>
#include <stddef.h>

void* memcpy(void* dest, const void* source, size_t num)
{
	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)source;
	
	for (size_t i = 0 ; i < num ; i++)
	{
		d[i] = s[i];
	}
	
	return dest;
}