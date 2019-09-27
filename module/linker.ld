/* Define the entry point to be _start, so the bootloader will look for _start (defined in boot.s) and run it*/
ENTRY(_loader)
OUTPUT_FORMAT(elf32-i386)

REAL_BASE = 0x00008000;

/* lets start by defining all the sections */
SECTIONS
{
	. = 0xC0100000;
    __KERNEL_BEGIN = .;

	.text ALIGN(0x1000) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.multiboot)
	}

    __physreal_diff = . - REAL_BASE;
	.realmode REAL_BASE : AT(ADDR(.realmode) + __physreal_diff - 0xC0000000)
	{
        __realmode_vma_start = .;
        __realmode_lma_start = LOADADDR(.realmode) + 0xC0000000;

        *(.text.realmode);
        *(.data.realmode);
	}
	. = ALIGN(4);
    __realmode_vma_end = .;
    __realmode_secsize   = ((__realmode_vma_end)-(__realmode_vma_start));
    __realmode_secsize_l = __realmode_secsize>>2;
    __realmode_lma_end   = __realmode_vma_start + __physreal_diff + __realmode_secsize;

	. += __physreal_diff + REAL_BASE;

	.text ALIGN(0x1000) : AT(ADDR(.text) - 0xC0000000)
	{
   		*(.init);
   		*(.fini);
		EXCLUDE_FILE (build/smp_trampoline.o) *(.text*);
		*(.rodata);
	}

   	.data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000) {
       *(.data);
   	}
   
   	.eh_frame ALIGN (0x1000) : AT(ADDR(.eh_frame) - 0xC0000000) {
   		*(.eh_frame*);
   	}

	/* Read-write data (uninitialized) and stack */
   .bss ALIGN(0x1000) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON);
		*(.bss);
		*(.bootstrap_stack);
		*(.esp0_stack);
	}

	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
	__KERNEL_END_VIRTUAL = .;
	__KERNEL_END_PHYSICAL = . - 0xC0000000;

	/DISCARD/ : {
        *(.note.gnu.build-id);
        *(.comment);
    }
}
