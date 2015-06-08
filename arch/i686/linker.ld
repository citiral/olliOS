/* Define the entry point to be _start, so the bootloader will look for _start (defined in boot.s) and run it*/
ENTRY(_start)

/* lets start by defining all the sections */
SECTIONS
{
	/* we begin putting sections at the 1MB mark since that is where bootloaders usually search */
	. = 1M;

	/* As said in boot.s, the multiboot header should come first, so lets do that */
	.text BLOCK(4K) : ALIGN(4k)
	{
		*(.multiboot)
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
		*(.esp0_stack)
	}

	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}