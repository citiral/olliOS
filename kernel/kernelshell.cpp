//
// Created by Olivier on 30/09/16.
//

#include "kernelshell.h"
#include "alloc.h"
#include "devicemanager.h"
#include "devices/keyboard.h"
#include "fs/filesystem.h"
#include "fs/virtualfilesystem.h"
#include <string.h>
#include "stdio.h"

KernelShell::KernelShell(): _commands(), _env()
{
#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
	_commands.push_back(std::pair<const char *, CommandFunction>("allocinfo", &KernelShell::allocinfo));
	_commands.push_back(std::pair<const char *, CommandFunction>("allocmerge", &KernelShell::allocmerge));
#endif
    _commands.push_back(std::pair<const char*, CommandFunction>("devicesinfo", &KernelShell::devicesinfo));
    _commands.push_back(std::pair<const char*, CommandFunction>("help", &KernelShell::help));
    _commands.push_back(std::pair<const char*, CommandFunction>("ls", &KernelShell::ls));
	/*_commands.push_back(std::pair<const char*, CommandFunction>("cat", &KernelShell::cat));
	_commands.push_back(std::pair<const char*, CommandFunction>("cd", &KernelShell::cd));
	_commands.push_back(std::pair<const char*, CommandFunction>("set", &KernelShell::set));
	_commands.push_back(std::pair<const char*, CommandFunction>("unset", &KernelShell::unset));
	_env.set("pwd", "/");
	_env.set("home", "/");*/
}

#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
void KernelShell::allocinfo(Environment& env, std::vector<std::string> args)
{
	UNUSED(env);
	UNUSED(args);
	kernelAllocator.printStatistics();
	printf("\n");
}

void KernelShell::allocmerge(Environment& env, std::vector<std::string> args)
{
	UNUSED(env);
	UNUSED(args);
	kernelAllocator.merge();
}
#endif

void KernelShell::devicesinfo(Environment& env, std::vector<std::string> args)
{
	UNUSED(env);
	UNUSED(args);
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

void KernelShell::cat(Environment& env, std::vector<std::string> args)
{
    if (args.size() < 2) {
        printf("Please specify a directory.");
        return;
    }

    BlockDevice* file = vfs->openFile(Files::getPath(env, args[1]).c_str());

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

void KernelShell::ls(Environment& env, std::vector<std::string> args)
{
	DirEntry* dir;
	if (args.size() > 1)
	{
		dir = vfs->fromPath(Files::getPath(env, args[1].c_str()).c_str());
	}
	else
	{
		dir = vfs->fromPath(env.get("pwd").c_str());
	}

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

void KernelShell::cd(Environment& env, std::vector<std::string> args)
{
	UNUSED(env);
	if (args.size() <= 1)
		env.set("pwd", Files::normalize(env.get("home")));
	else
		env.set("pwd", Files::getPath(env, args[1].c_str()));
}

void KernelShell::help(Environment& env, std::vector<std::string> args)
{
	UNUSED(env);
	UNUSED(args);
	printf("Possible commands: ");

	for (size_t i = 0; i + 1 < _commands.size(); i++)
	{
		printf("%s, ", _commands[i].first);
	}

	if (_commands.size() > 0)
	{
		printf("%s", _commands.back().first);
	};
	printf("\n");
}

void KernelShell::set(Environment& env, std::vector<std::string> args)
{
	if (args.size() == 2)
	{
		const std::string var = args[1];
		if (env.isset(var))
			printf("%s=%s\n", var.data(), env.get(var).data());
		else
			printf("%s is not set\n", var.data());
	}
	else if (args.size() == 3)
	{
		const std::string var = args[1];
		const std::string val = args[2];
		env.set(var, val);
	}
}

void KernelShell::unset(Environment& env, std::vector<std::string> args)
{
	if (args.size() == 2)
	{
		const std::string var = args[1];
		if (env.isset(var))
			env.unset(var);
		else
			printf("%s was not set\n", var.data());
	}
}

void KernelShell::enter()
{
	printf("%s$ ", _env.get("pwd").data());

	// keep in the shell forever (unless we tell it to exit)
	while (true)
	{
		// read some virtual key events from the keyboard driver
		VirtualKeyEvent input[10];
		size_t read = ((KeyboardDriver*) deviceManager.getDevice(DeviceType::Keyboard, 0))->read(input, 10);

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
				for (size_t i = 0; i < _commands.size(); i++)
				{
					if (split[0] == _commands[i].first)
					{
						notFound = false;
						(this->*_commands[i].second)(_env, split);
						break;
					}
				}
				// otherwise print a command not found
				if (notFound)
				{
					printf("Unknown command: %s\n", line.data());
				}
			}
			printf("%s$ ", _env.get("pwd").data());
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