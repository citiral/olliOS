#include "pic.h"
#include "io.h"

//ends an interrupt comming from the PIC
void endInterrupt(u8 irq)
{
	//io::inb(SLAVE_COMMAND);
	//io::inb(MASTER_COMMAND);
	//if the irq is coming from the slave, send the command to him
	if (irq >= 8) {
		outb(SLAVE_COMMAND, EOI);
	}
	//and send the command to the master
	outb(MASTER_COMMAND, EOI);	
}

//clears all interrupt masks of the PIC
void clearAllMasks()
{
	outb(MASTER_DATA, 0);
	outb(SLAVE_DATA, 0);
}

void enableIrq(u8 irq)
{
	u8 mask;
	if (irq >= 8)
		mask = inb(SLAVE_DATA);
	else
		mask = inb(MASTER_DATA);
	
	mask = mask & (0xFFFF ^ (0x0001 << irq));
	
	if (irq >= 8) {
		outb(SLAVE_DATA, mask);
	} else {
		outb(MASTER_DATA, mask);
	}
}

void disableIrq(u8 irq)
{
	u8 mask;
	if (irq >= 8) {
		mask = inb(SLAVE_DATA);
	} else {
		mask = inb(MASTER_DATA);
	};
	
	mask = mask | (0x0001 << irq);
	
	if (irq >= 8) {
		outb(SLAVE_DATA, mask);
	} else {
		outb(MASTER_DATA, mask);
	};
}

void mapPics(u8 masterOffset, u8 slaveOffset)
{
	//first get the old maps
	u8 a1 = inb(MASTER_DATA);
	u8 a2 = inb(SLAVE_DATA);

	//begin the initialisation
	outb(MASTER_COMMAND, INIT);
	outb(SLAVE_COMMAND, INIT);

	//send the offsets
	outb(MASTER_DATA, masterOffset);
	outb(SLAVE_DATA, slaveOffset);

	//tell master it has a slave
	outb(MASTER_DATA, 4);
	//tell slave the cascade identity(??)
	outb(SLAVE_DATA, 2);

	//and the last value that always needs to be send
	outb(MASTER_DATA, ICW4_8086);
	outb(SLAVE_DATA, ICW4_8086);

	//and restore the old masks
	outb(MASTER_DATA, a1);
	outb(SLAVE_DATA, a2);
}
//initializes the pic and allows all interrupts
//this also remaps the the pic so it doesn't conflict with intel reserved interrupts
void PicInit()
{
	mapPics(0x20, 0x28);
	clearAllMasks();
	disableIrq(0);

	for (u8 x = 0 ; x <= 16 ; x++) {
		endInterrupt(x);
	}

	__asm__ volatile("sti");
}