# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard. We put this into a special
# section so we can force the header to be in the start of the final program.
# You don't need to understand all these details as it is just magic values that
# is documented in the multiboot standard. The bootloader will search for this
# magic sequence and recognize us as a multiboot kernel.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Currently the stack pointer register (esp) points at anything and using it may
# cause massive harm. Instead, we'll provide our own stack. We will allocate
# room for a small temporary stack by creating a symbol at the bottom of it,
# then allocating 16384 bytes for it, and finally creating a symbol at the top.
.section .bootstrap_stack, "aw", @nobits
.global stack_top
stack_bottom:
.skip 16384 # 16 KiB
stack_top:


#a stack that can be used as the ESP0 stack when interrupted
.section .esp0_stack, "aw", @nobits
.global esp0_top
esp0_bottom:
.skip 16384 # 16 KiB
esp0_top:

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded. It
# doesn't make sense to return from this function as the bootloader is gone.

.set VIRTUAL_OFFSET, 0xC0000000
.set VIRTUAL_PAGE_INDEX, VIRTUAL_OFFSET >> 22

# allocate a page directory purely for bootstrapping the higher half kernel.
# For memory reasons bootstrapping happens with 4MB pages

.section .data
.align 0x1000
_bootstrap_page_directory:
	.long 0x00000083 # memory map first mb
	.rept VIRTUAL_PAGE_INDEX - 1
	.long 0 # all 0 entries untill kernel
	.endr
	.long 0x00000083 # the memory directory of the higher half kernel
	.rept 1024 - VIRTUAL_PAGE_INDEX - 1
	.long 0 # all 0 entries untill kernel
	.endr

.section .text
.global loader
.equ loader, _loader - 0xC0000000
# higher half kernel bootstrap loader
_loader:
	# use the bootstrap page directory
	mov $_bootstrap_page_directory - VIRTUAL_OFFSET, %eax
	mov %eax, %cr3
	mov $0x1234, %eax

	# enable page size extensions

	mov %cr4, %eax
	or $0x00000010, %eax
	mov %eax, %cr4

	# enable paging
	mov %cr0, %eax
	or $0x80000000, %eax
	mov %eax, %cr0

	#jump to start with absolute jump, for paging to kick in
	movl $_start, %ecx
	jmp *%ecx

_start:
	# Welcome to kernel mode! We now have sufficient code for the bootloader to
	# load and run our operating system. It doesn't do anything interesting yet.
	# Perhaps we would like to call printf("Hello, World\n"). You should now
	# realize one of the profound truths about kernel mode: There is nothing
	# there unless you provide it yourself. There is no printf function. There
	# is no <stdio.h> header. If you want a function, you will have to code it
	# yourself. And that is one of the best things about kernel development:
	# you get to make the entire system yourself. You have absolute and complete
	# power over the machine, there are no security restrictions, no safe
	# guards, no debugging mechanisms, there is nothing but what you build.

	# By now, you are perhaps tired of assembly language. You realize some
	# things simply cannot be done in C, such as making the multiboot header in
	# the right section and setting up the stack. However, you would like to
	# write the operating system in a higher level language, such as C or C++.
	# To that end, the next task is preparing the processor for execution of
	# such code. C doesn't expect much at this point and we only need to set up
	# a stack. Note that the processor is not fully initialized yet and stuff
	# such as floating point instructions are not available yet.

	# To set up a stack, we simply set the esp register to point to the top of
	# our stack (as it grows downwards).
	movl $stack_top, %esp
	# We are now ready to actually execute C code. We cannot embed that in an
	# assembly file, so we'll create a kernel.c file in a moment. In that file,
	# we'll create a C entry point called kernel_main and call it here.
	call _init
	call main
	call _fini

	# In case the function returns, we'll want to put the computer into an
	# infinite loop. To do that, we use the clear interrupt ('cli') instruction
	# to disable interrupts, the halt instruction ('hlt') to stop the CPU until
	# the next interrupt arrives, and jumping to the halt instruction if it ever
	# continues execution, just to be safe. We will create a local label rather
	# than real symbol and jump to there endlessly.
	cli
	hlt
.Lhang:
	jmp .Lhang

# Set the size of the _start symbol to the current location '.' minus its start.
# This is useful when debugging or when you implement call tracing.
.size _start, . - _start
