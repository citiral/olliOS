#include <stdio.h>


extern "C" int module_load(void)
{
    printf("Hello world from module!\n");
    while(1);
    return 0;
}
