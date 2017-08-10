//
// Created by Olivier on 30/09/16.
//

#include "kernelshell.h"
#include "alloc.h"
#include "devicemanager.h"
#include "fs/virtualfilesystem.h"
#include <string.h>

KernelShell::KernelShell() {
#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
    _commands.push_back(std::pair<const char*, CommandFunction>("allocinfo", &KernelShell::allocinfo));
    _commands.push_back(std::pair<const char*, CommandFunction>("allocmerge", &KernelShell::allocmerge));
#endif
    _commands.push_back(std::pair<const char*, CommandFunction>("devicesinfo", &KernelShell::devicesinfo));
    _commands.push_back(std::pair<const char*, CommandFunction>("help", &KernelShell::help));
    _commands.push_back(std::pair<const char*, CommandFunction>("ls", &KernelShell::ls));
}

#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
void KernelShell::allocinfo(const char* cmd) {
    kernelAllocator.printStatistics();
    printf("\n");
}

void KernelShell::allocmerge(const char* cmd) {
    kernelAllocator.merge();
}
#endif

void KernelShell::devicesinfo(const char* cmd) {
    /*const char* names[] = {"Keyboard", "Screen", "Storage"};

    for (u8 i = 0 ; i < 3 ; i++) {
        DeviceType type = (DeviceType)i;
        printf("%s: ", names[i]);

        for (int i = 0 ; i + 1 < deviceManager.getDevices(type).size() ; i++) {
            printf("%s, ", deviceManager.getDevices(type)[i]->getDeviceName());
        }
        if (deviceManager.getDevices(type).size() > 0)
            printf("%s", deviceManager.getDevices(type).back()->getDeviceName());
        printf("\n");
    }*/
}

void KernelShell::ls(const char* cmd) {
    DirEntry* dir = vfs->fromPath(cmd+3);

    if (!dir) {
        printf("Invalid directory %s\n", cmd+3);
        return;
    }

    while (dir->valid()) {
        printf("%s\n", dir->name().c_str());
        dir->advance();
    }

    delete dir;
}

void KernelShell::help(const char* cmd) {
    printf("Possible commands: ");

    for (int i = 0 ; i + 1 < _commands.size() ; i++) {
        printf("%s, ", _commands[i].first);
    }

    if (_commands.size() > 0) {
        printf("%s", _commands.back().first);
    }
    ;
    printf("\n");
}

void KernelShell::enter() {
    printf("> ");

    // keep in the shell forever (unless we tell it to exit)
    while (true) {
        // read some virtual key events from the keyboard driver
        VirtualKeyEvent input[10];
        int read = deviceManager.getDevice(DeviceType::Keyboard, 0)->read(input, 10);

        // send them to the input formatter
        for (size_t i = 0 ; i < read ; i += sizeof(VirtualKeyEvent))
        {
            _input.handleVirtualKeyEvent(input[i]);
        }

        // if there is a line, fetch it (for now use a std::vector<char> since we don't have strings yet)
        if (_input.isLineReady()) {
            std::vector<char> line = _input.getNextLine();

            // find the first matching command and call it
            bool notFound = true;

            for (int i = 0 ; i < _commands.size() ; i++) {
                if (strlen(_commands[i].first) <= strlen(line.data())) {
                    int len = strlen(_commands[i].first);
                    bool matches = true;
                    for (int k = 0 ; k < len ; k++) {
                        matches &= (_commands[i].first[k] == line[k]);
                    }
                
                    if (matches) {
                        (this->*_commands[i].second)(line.data());
                        notFound = false;
                    }
                }
            }

            // otherwise print a command not found
            if (notFound) {
                printf("unknown command: %s\n", line.data());
            }

            printf("> ");
        }

        __asm__ volatile("hlt");
    }
}