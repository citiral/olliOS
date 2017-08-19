#ifndef OLLIOS_CPU_H
#define OLLIOS_CPU_H

namespace CPU
{
	void dumpstack(unsigned int maxFrames);
	// Just panic
	void panic();
}

#endif