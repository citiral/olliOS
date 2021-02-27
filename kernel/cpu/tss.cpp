#include "tss.h"
#include "stdlib.h"
#include "memory/virtual.h"

using namespace tss;

TaskStateSegment* tss::allocate_tss_with_stack(size_t stack_size_in_pages)
{
    void* stack = memory::kernelPageDirectory.bindFirstFreeVirtualPages((void*)0xD0000000, stack_size_in_pages, memory::UserMode::Supervisor);
    if (stack == nullptr) {
        return nullptr;
    }

    TaskStateSegment* tss = new TaskStateSegment();
    if (tss == nullptr) {
        return nullptr;
    }

    tss->SS0 = 0x10;
    tss->ESP0 = (unsigned int) stack;

    return tss;
}

void tss::use_tss_at_index(u32 index)
{
    u16 op = index;
    __asm__ volatile("ltr %0"::"A"(op));
}