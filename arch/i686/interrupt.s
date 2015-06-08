#this file descripes all interrupt handlers
#they directly correspond to the indexes of the IDT



#this function is called when an interrupt is thrown that does not exist
.global int_unused
.align 4
int_unused:
	pushal
	call rust_int_unused
	popal
	iret
