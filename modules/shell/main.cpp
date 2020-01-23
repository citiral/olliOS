#include "types.h"
#include "kernelshell.h"
#include "fs/bindings.h"
#include <stdio.h>

using namespace bindings;

KernelShell* shell;

extern "C" void module_load(Binding* root)
{
    shell = new KernelShell();

    root->get("keyboard")->on_data([](Binding* keyboard, size_t count, const void* data) {
        const VirtualKeyEvent* keys = (VirtualKeyEvent*) data;

        for (int i = 0 ; i < count / sizeof(VirtualKeyEvent) ; i++) {
            shell->enter(keys[i]);
        }
        
        return true;
    });
}
