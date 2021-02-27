#ifndef _CPU_UTIL_H_
#define _CPU_UTIL_H_

#include "types.h"

#define IA32_SYSENTER_CS 0x174
#define IA32_SYSENTER_ESP 0x175
#define IA32_SYSENTER_EIP 0x176

extern "C" {
    void write_model_specific_register(u32 reg, u32 high, u32 low);
    u32 read_model_specific_register_lower(u32 reg);
    u32 read_model_specific_register_higher(u32 reg);
}

#endif