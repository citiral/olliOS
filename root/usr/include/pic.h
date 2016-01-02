#pragma once
#include "types.h"

#define MASTER_COMMAND 0x20
#define MASTER_DATA 0x21
#define SLAVE_COMMAND 0xA0
#define SLAVE_DATA 0xA1

//end of interrupt command for the PIC
#define EOI 0x20
#define INIT 0x11
#define ICW4_8086 0x01

//ends an interrupt comming from the PIC
void endInterrupt(u8 irq);
//clears all interrupt masks of the PIC
void clearAllMasks();
void enableIrq(u8 irq);
void disableIrq(u8 irq);
void mapPics(u8 masterOffset, u8 slaveOffset);
//initializes the pic and allows all interrupts
//this also remaps the the pic so it doesn't conflict with intel reserved interrupts
void PicInit();
