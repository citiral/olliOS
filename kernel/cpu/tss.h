#ifndef _TSS_H_
#define _TSS_H_

#include "types.h"
#include "cdefs.h"

namespace tss {

class __attribute__((packed)) TaskStateSegment {
public:
    u16 LINK;
    u16 _reserved0;
    u32 ESP0;
    u16 SS0;
    u16 _reserved1;
    u32 ESP1;
    u16 SS1;
    u16 _reserved2;
    u32 ESP2;
    u16 SS2;
    u16 _reserved3;
    u32 CR3;
    u32 EIP;
    u32 EFLAGS;
    u32 EAX;
    u32 ECX;
    u32 EDX;
    u32 EBX;
    u32 ESP;
    u32 EBP;
    u32 ESI;
    u32 EDI;
    u16 ES;
    u16 _reserved4;
    u16 CS;
    u16 _reserved5;
    u16 SS;
    u16 _reserved6;
    u16 DS;
    u16 _reserved7;
    u16 FS;
    u16 _reserved8;
    u16 GS;
    u16 _reserved9;
    u16 LDTR;
    u16 _reserved10;
    u16 _reserved11;
    u16 IOPB;
};

TaskStateSegment* allocate_tss_with_stack(size_t stack_size_in_pages);
void use_tss_at_index(u32 index);

}

#endif