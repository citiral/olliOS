//
// Created by citiral on 9/23/16.
//

#ifndef OLLIOS_GIT_LINKER_H
#define OLLIOS_GIT_LINKER_H

// two sentinels defined by the linker script
// these are the start and end positions of the kernel in virtual memory
// (the values are 0xC0000000+ but the kernel starts from 0x00000000)
extern size_t __KERNEL_BEGIN;
extern size_t __KERNEL_END;

#define KERNEL_BEGIN (&__KERNEL_BEGIN)
#define KERNEL_END (&__KERNEL_END)

#endif //OLLIOS_GIT_LINKER_H
