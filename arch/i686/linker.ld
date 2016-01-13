/* Define the entry point to be _start, so the bootloader will look for _start (defined in boot.s) and run it*/
ENTRY(_loader)
OUTPUT_FORMAT(elf32-i386)

/* lets start by defining all the sections */
SECTIONS
{
	. = 0xC0100000;

	/* As said in boot.s, the multiboot header should come first, so lets do that */
	.text ALIGN(0x1000) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.multiboot)
		*(.text)
		*(.rodata)
	}

   .data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000) {
       *(.data)
   }

	/* Read-write data (uninitialized) and stack */
  .bss ALIGN(0x1000) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
		*(.esp0_stack)
	}

	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
	KERNEL_END = .;
}
