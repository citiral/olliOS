#include <abi.h>

void __cxa_pure_virtual(void)
{
    #ifdef __is_libkpp
    //TODO panic
    #elif __is_libcpp
    //TODO close program
    #endif
}
