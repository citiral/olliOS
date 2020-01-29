#include <string.h>
#include <stddef.h>

int memcmp(const void* ptr1, const void* ptr2, size_t num)
{
	const unsigned char* a = (const unsigned char*) ptr1;
	const unsigned char* b = (const unsigned char*) ptr2;

	for (size_t i = 0 ; i < num ; i++)
	{
		int diff = (int)a[i] - (int)b[i];

		if (diff != 0)
			return diff;
	}

	return 0;
}