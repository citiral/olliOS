#include "tss.h"
#include "stdlib.h"
#include "memory/virtual.h"

using namespace tss;

TaskStateSegment* tss::allocate_tss()
{
    TaskStateSegment* tss = new TaskStateSegment();
    if (tss == nullptr) {
        return nullptr;
    }

    tss->SS0 = 0x10;
    tss->ESP0 = 0;
    tss->IOPB = sizeof(TaskStateSegment);
    return tss;
}

void tss::use_tss_at_index(u32 index)
{
    u16 op = index;
    __asm__ volatile("ltr %0"::"A"(op));
}
