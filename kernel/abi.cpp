#include "abi.h"

void __cxa_pure_virtual(void)
{
    #ifdef __is_libkpp
    //TODO panic
    #elif __is_libcpp
    //TODO close program
    #endif
}

void __cxa_atexit(void (*func) (void *) __attribute__((unused)), void* arg __attribute__((unused)), void* dso_handle __attribute__((unused)))
{
    //TODO: veeeeeeeery important, implement this :) prob requires memory allocation
}
