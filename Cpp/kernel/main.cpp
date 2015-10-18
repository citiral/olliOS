#include "types.h"
#include "vga.h"

extern "C" void main() {
	VgaClear();
	VgaWriteChars("Welcome to OlliOS!");
}