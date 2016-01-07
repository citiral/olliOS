#include "pagealloc.h"
#include "stdio.h"
#include "string.h"

void initializeEntry(PageEntry& entry)
{
    //set the whole struct to 0
    
    for (size_t i = 0 ; i < 1024 ; i++)
    {
        printf("%d ", entry.pages[i]);

    }
}
