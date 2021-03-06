//
// Created by Olivier on 30/09/16.
//

#include "kernelshell.h"
#include "linker.h"
#include "memory/alloc.h"
#include "memory/virtual.h"
#include "memory/physical.h"
#include "keyboard/keyboard.h"
#include "threading/thread.h"
#include "threading/scheduler.h"
#include "cpu/apic.h"
#include <string.h>
#include "stdio.h"
#include "elf/elf.h"
#include "virtualfile.h"

#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
void allocinfo(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	UNUSED(args);
	kernelAllocator.printStatistics();
	printf("\n");
}

void allocmerge(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	UNUSED(args);
	kernelAllocator.merge();
}
#endif

void cat(KernelShell* shell, std::vector<std::string>* args)
{
	fs::File* file;
	
	if (args->size() > 1) {
		file = shell->working_directory->get(args->at(1).c_str());
	} else {
		file = shell->working_directory;
	}

	if (file == NULL && args->size() > 1) {
		printf("Invalid path: %s\n", args->at(1).c_str());
		return;
	}

    char buffer[2048];
	size_t offset = 0;
	int read;

	fs::FileHandle* handle = file->open();
	do {
		read = handle->read(buffer, sizeof(buffer), offset);
		for (int i = 0 ; i < read ; i++)
			putchar(buffer[i]);
		offset += read;
	} while (read > 0);

	handle->close();
}

void stat(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	UNUSED(args);
	/*bindings::Binding* bind;
	
	if (args->size() > 1) {
		bind = shell->working_directory->get(args->at(1).c_str());
	} else {
		bind = shell->working_directory;
	}

	if (bind == NULL && args->size() > 1) {
		printf("Invalid path: %s\n", args->at(1).c_str());
		return;
	}

	printf("Size: %d\n", bind->get_size());*/
}

void hex(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	UNUSED(args);
	/*bindings::Binding* bind;
	
	if (args->size() > 1) {
		bind = shell->working_directory->get(args->at(1).c_str());
	} else {
		bind = shell->working_directory;
	}

	if (bind == NULL && args->size() > 1) {
		printf("Invalid path: %s\n", args->at(1).c_str());
		return;
	}

    u8 buffer[64];
	size_t total = 0;
	size_t read;
	do {
		read = bind->read(buffer, sizeof(buffer), total);
		for (size_t i = 0 ; i < read ; i++) {
			printf("%.2X", (unsigned int) buffer[i]);;
		}
		total += read;
	} while (read > 0);

	putchar('\n');*/
}

void ls(KernelShell* shell, std::vector<std::string>* args)
{
	fs::File* folder;
	
	if (args->size() > 1) {
		folder = shell->working_directory->get(args->at(1).c_str());
	} else {
		folder = shell->working_directory;
	}

	if (folder == NULL && args->size() > 1) {
		printf("Invalid path: %s\n", args->at(1).c_str());
		return;
	}


    fs::FileHandle* handle = folder->open();
	if (handle) {
		fs::File* child;
		while ((child = handle->next_child()) != NULL) {
			printf("%s\n", child->get_name());
		}
	}
	handle->close();
}

void cd(KernelShell* shell, std::vector<std::string>* args)
{
	fs::File* file;
	
	if (args->size() > 1) {
		file = shell->working_directory->get(args->at(1).c_str());
	} else {
		file = fs::root;
	}


	if (file == NULL && args->size() > 1) {
		printf("Invalid path: %s\n", args->at(1).c_str());
		return;
	}

	shell->working_directory = file;
}

void touch(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	UNUSED(args);
	/*bindings::Binding* bind = nullptr;
	
	if (args->size() != 2) {		
		printf("Binding name expected\n", args->at(1).c_str());
		return;
	}
	
	bind = shell->working_directory->get(args->at(1).c_str());

	if (bind != nullptr) {
		printf("Bind already exists\n");
		return;
	}

	shell->working_directory->add(new bindings::OwnedBinding(args->at(1)));*/
}

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
}

void run(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	UNUSED(args);
	if (args->size() < 2)
	{
		printf("Usage: load program [args...]\n");
		return;
	}

	fs::File* file = shell->working_directory->get(args->at(1).c_str());

	if (file == NULL) {
		printf("Invalid path: %s\n", args->at(1).c_str());
		return;
	}

	shell->runningProcessStdin = new fs::Stream("stdin", 64);

	Process* p = new Process();
	std::string wd = "/root/usr/bin";
	p->set_arguments(*args);
	p->init(file, wd);

    p->open(shell->runningProcessStdin, 0, 0);
    p->open("/sys/vga", 0, 0);
    p->open("/sys/vga", 0, 0);

	p->start();

	shell->runningProcess = p;
}

void load(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	UNUSED(args);
	/*if (args->size() < 2)
	{
		printf("Usage: load program\n");
		return;
	}

	bindings::Binding* bind = shell->working_directory->get(args->at(1).c_str());
	if (bind == NULL) {
		printf("Invalid path: %s\n");
		return;
	}

	size_t filesize = bind->get_size();
	if (filesize == 0) {
		printf("Filesize is 0.\n");
		return;
	}

	u8* buffer = new u8[filesize];
	size_t total = 0;

	do {
		size_t read = bind->read((void*)(buffer + total), filesize - total, total);
		total += read;
	} while (total != filesize);

	elf::elf* e = new elf::elf(buffer, true);
	if (e == nullptr) {
		printf("Failed allocating elf\n");
		while (1);
	}

	if (e->link_as_kernel_module(*symbolMap) != 0 && 0) {
		printf("failed linking elf\n");
	} else {
		std::string argv = "";

		for (int i = 1 ; i < args->size() ; i++) {
			argv += args->at(i);
			argv += " ";
		}

		void (*module_load)(bindings::Binding*, const char*);
		e->get_symbol_value("module_load", (u32*) &module_load);
		module_load(bindings::root, argv.c_str());
	}*/
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

void KernelShell::prompt()
{
	printf("> ");
}

KernelShell::KernelShell(): _commands()
{
    printf("hello world from kernshell\n");
#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
	_commands.push_back(std::pair<const char *, CommandFunction>("allocinfo", &allocinfo));
	_commands.push_back(std::pair<const char *, CommandFunction>("allocmerge", &allocmerge));
#endif
    _commands.push_back(std::pair<const char*, CommandFunction>("help", &help));
    _commands.push_back(std::pair<const char*, CommandFunction>("ls", &ls));
	_commands.push_back(std::pair<const char*, CommandFunction>("cat", &cat));
	//_commands.push_back(std::pair<const char*, CommandFunction>("stat", &stat));
	//_commands.push_back(std::pair<const char*, CommandFunction>("hex", &hex));
	_commands.push_back(std::pair<const char*, CommandFunction>("cd", &cd));
	//_commands.push_back(std::pair<const char*, CommandFunction>("touch", &touch));
	//_commands.push_back(std::pair<const char*, CommandFunction>("load", &load));
	_commands.push_back(std::pair<const char*, CommandFunction>("run", &run));

	//working_directory = fs::root->get("dev/ata0/root/boot");
	working_directory = fs::root;
	prompt();
}

void KernelShell::enter(VirtualKeyEvent input)
{
	// read some virtual key events from the keyboard driver
	//VirtualKeyEvent input[10];
	//size_t read = ((keyboard::KeyboardDriver*) deviceManager.getDevice(DeviceType::Keyboard, 0))->read(input, 10);

    // send them to the input formatter
	_input.handleVirtualKeyEvent(input);

	if (_input.isLineReady())
	{
		std::string line = _input.getNextLine();

		if (runningProcess && runningProcess->state != ProcessState::Stopped)
		{
			int length = line.length();
			int written = 0;

			while (written < length) {
				written += runningProcess->write(0, line.c_str() + written, length - written);
			}
			runningProcess->write(0, "\n", 1);
		} else {
			if (runningProcess) {
				delete runningProcess;
				delete runningProcessStdin;
				runningProcess = nullptr;
				runningProcessStdin = nullptr;
			}
			runCommand(line);
		}
    }
}

void KernelShell::runCommand(std::string command)
{

        // find the first matching command and call it
        bool notFound = true;

        std::vector<std::string> split = splitCommand(command);

		if (split.size() > 0)
		{
			for (size_t i = 0; i < _commands.size(); i++)
			{
				if (split.at(0) == _commands[i].first)
				{
					notFound = false;
					_commands[i].second(this, &split);
					prompt();
					break;
				}
			}
			// otherwise print a command not found
			if (notFound)
			{
				printf("Unknown command: %s\n", command.data());
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
