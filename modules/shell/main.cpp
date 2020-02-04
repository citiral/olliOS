#include "types.h"
#include "kernelshell.h"
#include "bindings.h"
#include "threading/scheduler.h"
#include <stdio.h>

using namespace bindings;

KernelShell* shell;


void test() {
    while (true) {
        printf("boop\n");
        threading::exit();
    }
}

extern "C" void module_load(Binding* root)
{
    shell = new KernelShell();

    root->get("sys")->get("keyboard")->on_data([](Binding* keyboard, size_t count, const void* data) {
        const VirtualKeyEvent* keys = (VirtualKeyEvent*) data;

        for (int i = 0 ; i < count / sizeof(VirtualKeyEvent) ; i++) {
            shell->enter(keys[i]);
        }
        
        return true;
    });
}
