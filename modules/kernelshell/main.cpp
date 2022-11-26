#include "types.h"
#include "kernelshell.h"
#include "file.h"
#include "threading/scheduler.h"
#include <stdio.h>

using namespace fs;

KernelShell* shell;

void test() {
    while (true) {
        printf("boop\n");
        threading::exit();
    }
}

void ShellThread(FileHandle* keyboard) {
    while (1) {
        VirtualKeyEvent key;
        i32 ret;

        ret = keyboard->read(&key, sizeof(VirtualKeyEvent), 0);
        shell->enter(key);
    }
}

extern "C" void module_load(File* root, const char* argv)
{
    shell = new KernelShell();

    File* keyboard = root->get("sys/keyboard");    
	threading::scheduler->schedule(new threading::Thread(nullptr, nullptr, ShellThread, keyboard->open()));
}
