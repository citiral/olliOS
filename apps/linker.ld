/* Define the entry point to be _start, so the bootloader will look for _start (defined in boot.s) and run it*/
ENTRY(_loader)
OUTPUT_FORMAT(elf32-i386)

/* lets start by defining all the sections */
SECTIONS
{
	. = 0x40000000;

	.text ALIGN(0x1000) : AT(ADDR(.text) - 0xC0000000)
	{
   		*(.init);
   		*(.fini);
		*(.text*);
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
	
	/DISCARD/ : {
        *(.note.gnu.build-id);
        *(.comment);
    }
}
