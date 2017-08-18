//
// Created by Olivier on 30/09/16.
//

#include "kernelshell.h"
#include "alloc.h"
#include "devicemanager.h"
#include "fs/virtualfilesystem.h"
#include <string.h>
#include "stdio.h"

KernelShell::KernelShell()
{
#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
	_commands.push_back(std::pair<const char *, CommandFunction>("allocinfo", &KernelShell::allocinfo));
	_commands.push_back(std::pair<const char *, CommandFunction>("allocmerge", &KernelShell::allocmerge));
#endif
    _commands.push_back(std::pair<const char*, CommandFunction>("devicesinfo", &KernelShell::devicesinfo));
    _commands.push_back(std::pair<const char*, CommandFunction>("help", &KernelShell::help));
    _commands.push_back(std::pair<const char*, CommandFunction>("ls", &KernelShell::ls));
    _commands.push_back(std::pair<const char*, CommandFunction>("cat", &KernelShell::cat));
}

#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
void KernelShell::allocinfo(std::vector<std::string> args)
{
	kernelAllocator.printStatistics();
	printf("\n");
}

void KernelShell::allocmerge(std::vector<std::string> args)
{
	kernelAllocator.merge();
}
#endif

void KernelShell::devicesinfo(std::vector<std::string> args)
{
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

void KernelShell::cat(std::vector<std::string> args) {
    if (args.size() < 2) {
        printf("Please specify a directory.");
        return;
    }

    Stream* file = vfs->openFile(args[1].c_str());

    if (!file) {
        printf("Invalid file: %s\n", args[1].c_str());
        return;
    }

    while (true) {
        char data;
        bool read = file->read(&data, 1);

        if (read) {
            printf("%c", data);
        } else {
            break;
        }
    }

    delete file;
}

void KernelShell::ls(std::vector<std::string> args) {
    DirEntry* dir;
    if (args.size() > 1)
        dir = vfs->fromPath(args[1].c_str());
    else
        dir = vfs->getRoot();

    if (!dir && args.size() == 2) {
        printf("Invalid directory: %s\n", args[1].c_str());
        return;
    }

    if (dir) {
        while (dir->valid()) {
            printf("%s\n", dir->name().c_str());
            dir->advance();
        }

        delete dir;
    }
}

void KernelShell::help(std::vector<std::string> args)
{
	printf("Possible commands: ");

	for (int i = 0; i + 1 < _commands.size(); i++)
	{
		printf("%s, ", _commands[i].first);
	}

	if (_commands.size() > 0)
	{
		printf("%s", _commands.back().first);
	};
	printf("\n");
}

void KernelShell::enter()
{
	printf("> ");

	// keep in the shell forever (unless we tell it to exit)
	while (true)
	{
		// read some virtual key events from the keyboard driver
		VirtualKeyEvent input[10];
		int read = deviceManager.getDevice(DeviceType::Keyboard, 0)->read(input, 10);

		// send them to the input formatter
		for (size_t i = 0; i < read; i += sizeof(VirtualKeyEvent))
		{
			_input.handleVirtualKeyEvent(input[i]);
		}

		// if there is a line, fetch it (for now use a std::vector<char> since we don't have strings yet)
		if (_input.isLineReady())
		{
			std::string line = _input.getNextLine();

			// find the first matching command and call it
			bool notFound = true;

			std::vector<std::string> split = splitCommand(line);

			if (split.size() > 0)
			{
				for (int i = 0; i < _commands.size(); i++)
				{
					if (split[0] == _commands[i].first)
					{
						notFound = false;
						(this->*_commands[i].second)(split);
						break;
					}
				}
				// otherwise print a command not found
				if (notFound)
				{
					printf("Unknown command: %s\n", line.data());
				}
			}
			printf("> ");
		}

		__asm__ volatile("hlt");
	}
}

std::vector<std::string> KernelShell::splitCommand(std::string cmd)
{
	std::vector<std::string> out;

	// keep looping until the cmd is exhausted
	const char *cmdData = cmd.data();
	while (*cmdData != '\0')
	{
		// first we skip as much whitespace as possible
		while (*cmdData == ' ' || *cmdData == '\t')
			cmdData++;

		// and then we gobble as many characters into the current command as possible
		int length = 0;
		while (cmdData[length] != ' ' && cmdData[length] != '\t' && cmdData[length] != '\0')
			length++;

		if (length == 0)
			break;

		out.push_back(std::string(cmdData, length));

		cmdData += length;
		//TODO "
	}

	return out;
}