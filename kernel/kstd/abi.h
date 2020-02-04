#ifndef __ABI_H
#define __ABI_H

/**
 * Because we are C++, there are some abi functions that need to be implemented
 * to handle whatever unexpected (or expected) cases happen.
 * Normally a libc++ would handle this for us, but because we are the kernel,
 * and writing a libc++ is way too hardcore, we do it here. Also, those
 * function names look cool.
*/

//We declare them as extern "C" so no name mangling happens.
extern "C" {
    void __cxa_pure_virtual(void);
    void __cxa_atexit(void (*func) (void *) __attribute__((unused)), void* arg __attribute__((unused)), void* dso_handle __attribute__((unused)));
}

#endif /* end of include guard: __ABI_H */
