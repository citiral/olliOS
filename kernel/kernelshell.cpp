//
// Created by Olivier on 30/09/16.
//

#include "kernelshell.h"
#include "alloc.h"
#include "devicemanager.h"
#include "devices/keyboard.h"
#include "fs/filesystem.h"
#include "fs/virtualfilesystem.h"
#include "threading/thread.h"
#include "threading/scheduler.h"
#include "apic.h"
#include <string.h>
#include "stdio.h"

#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
void allocinfo(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	kernelAllocator.printStatistics();
	printf("\n");
	delete args;
}

void allocmerge(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	kernelAllocator.merge();
	delete args;
}
#endif

void devicesinfo(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
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
	delete args;
}

void cat(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	if (args->size() < 2) {
		printf("Please specify a directory.");
		return;
	}

	BlockDevice* file = vfs->openFile(args->at(1).c_str());

	if (!file) {
		printf("Invalid file: %s\n", args->at(1).c_str());
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
	delete args;
}

void ls(KernelShell* shell, std::vector<std::string>* args)
{
	//LOG_DEBUG("pwd: %s", threading::currentProcess()->env.get("pwd").c_str());
	UNUSED(shell);
	DirEntry* dir;
	if (args->size() > 1)
	{
		dir = vfs->fromPath(args->at(1).c_str());
	}
	else
	{
		dir = vfs->fromPath("/");
	}

	if (!dir && args->size() == 2) {
		printf("Invalid directory: %s\n", args->at(1).c_str());
		return;
	}

	if (dir) {
		while (dir->valid()) {
			printf("%s\n", dir->name().c_str());
			dir->advance();
		}

		delete dir;
	}
	delete args;
}
/*
void cd(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	if (args->size() <= 1)
		threading::currentProcess()->parent->env.set("pwd", Files::normalize(threading::currentProcess()->env.get("home")));
	else
		threading::currentProcess()->parent->env.set("pwd", Files::getPath(threading::currentProcess()->env, args->at(1).c_str()));
	delete args;
}*/

void help(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(args);
	printf("Possible commands: ");

	for (size_t i = 0; i + 1 < shell->commands().size(); i++)
	{
		printf("%s, ", shell->commands()[i].first);
	}

	if (shell->commands().size() > 0)
	{
		printf("%s", shell->commands().back().first);
	};
	printf("\n");
	delete args;
}
/*
void set(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	if (args->size() == 2)
	{
		const std::string var = args->at(1);
		if (threading::currentProcess()->env.isset(var))
			printf("%s=%s\n", var.data(), threading::currentProcess()->env.get(var).data());
		else
			printf("%s is not set\n", var.data());
	}
	else if (args->size() == 3)
	{
		const std::string var = args->at(1);
		const std::string val = args->at(2);
		threading::currentProcess()->env.set(var, val);
	}
	delete args;
}

void unset(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	if (args->size() == 2)
	{
		const std::string var = args->at(1);
		if (threading::currentProcess()->env.isset(var))
			threading::currentProcess()->env.unset(var);
		else
			printf("%s was not set\n", var.data());
	}
	delete args;
}*/

KernelShell::KernelShell(): _commands()
{
#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
	_commands.push_back(std::pair<const char *, CommandFunction>("allocinfo", &allocinfo));
	_commands.push_back(std::pair<const char *, CommandFunction>("allocmerge", &allocmerge));
#endif
    _commands.push_back(std::pair<const char*, CommandFunction>("devicesinfo", &devicesinfo));
    _commands.push_back(std::pair<const char*, CommandFunction>("help", &help));
    _commands.push_back(std::pair<const char*, CommandFunction>("ls", &ls));
	_commands.push_back(std::pair<const char*, CommandFunction>("cat", &cat));
	//_commands.push_back(std::pair<const char*, CommandFunction>("cd", &cd));
	/*_commands.push_back(std::pair<const char*, CommandFunction>("set", &set));
	_commands.push_back(std::pair<const char*, CommandFunction>("unset", &unset));
	_env.set("pwd", "/");
	_env.set("home", "/");*/
}
void KernelShell::enter()
{
	// keep in the shell forever (unless we tell it to exit)
	while (true)
	{
		// read some virtual key events from the keyboard driver
		keyboard::VirtualKeyEvent input[10];
		size_t read = ((keyboard::KeyboardDriver*) deviceManager.getDevice(DeviceType::Keyboard, 0))->read(input, 10);

		// send them to the input formatter
		for (size_t i = 0; i < read; i += sizeof(keyboard::VirtualKeyEvent))
		{
			_input.handleVirtualKeyEvent(input[i]);
		}

		// if there is a line, fetch it (for now use a std::vector<char> since we don't have strings yet)
		if (_input.isLineReady())
		{
			std::string line = _input.getNextLine();

			// find the first matching command and call it
			bool notFound = true;

			std::vector<std::string>* split = new std::vector<std::string>();
			*split = splitCommand(line);

			if (split->size() > 0)
			{
				for (size_t i = 0; i < _commands.size(); i++)
				{
					if (split->at(0) == _commands[i].first)
					{
						notFound = false;
						threading::Thread* p = new threading::Thread(this->_commands[i].second, this, split);
						threading::scheduler->schedule(p);
						break;
					}
				}
				// otherwise print a command not found
				if (notFound)
				{
					printf("Unknown command: %s\n", line.data());
				}
			}
		}
	}
}

std::vector<std::pair<const char*, KernelShell::CommandFunction>>& KernelShell::commands() {
	return _commands;
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