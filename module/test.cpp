#include <stdio.h>

extern "C" void testprint(const char*);

extern "C" int module_load(void)
{
    //putchar('a');
    printf("Hello world!\n");
    //while(1);
    return 10;
}
