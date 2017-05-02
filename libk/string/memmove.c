#include <string.h>
#include <stddef.h>

void* memmove(void* dest, const void* source, size_t num)
{
	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)source;

	if (source > dest)
	{
		for (size_t i = 0 ; i < num ; i++)
		{
			d[i] = s[i];
		}
	}
	else
	{
		for (size_t i = num ; i > 0 ; i--)
		{
			d[i-1] = s[i-1];
		}
	}

	return dest;
}