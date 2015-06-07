#we declare the multiboot header so the bootloader can find this and load the kernel
.set ALIGN,		1<<0			#align loaded modules on page boundaries
.set MEMINFO,	1<<1			#provide memory map
.set FLAGS, ALIGN | MEMINFO 	#we generaet the actual flags
.set MAGIC, 0x1BADB002			#magic number so the bootloader can find the header
.set CHECKSUM, -(MAGIC + FLAGS)	#checksum for the above flag, so we can really prove we are multiboot

#we put the multiboot data in a seperate section so the linker can put this on the top
#of the file, so the bootloader can find this section
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

#make a section for the stack
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # this is 16KB of stack
stack_top:


#the linker script will define _start to be the entry point of the kernel
#so the bootloader will jump to this section
.section .text
.global _start
.type _start, @function
_start: 
	#first we set up our stack register to the stack we previously allocated
	#the stack grows downwards so we move the top value of the stack
	movl $stack_top, %esp

	#now we jump into the actual entry point
	call main

	#if this ever returns we let the pc hang
	#first we disable interrupts
	cli
	#and then we halt so it does nothing
	hlt

	#if it ever continues, we hardwait anyway
.hang:
	jmp .hang

# Set the size of the _start symbol to the current location '.' minus the start location.
# This is useful when debugging or when you implement call tracing.
.size _start, . - _start

