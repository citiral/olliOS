#this file descripes all interrupt handlers
#they directly correspond to the indexes of the IDT

#this function is called when an interrupt is thrown that does not exist
#.global int_unused
#.align 4
#int_unused:
#	pushal
	#cdecl requires direction flag to be cleared
#	cld
#	call rust_int_unused
#	popal
#	iretl

#.global int_keyboard
#.align 4
#int_keyboard:
#	pushal
	#cdecl requires direction flag to be cleared
#	cld
#	call rust_int_keyboard
#	popal
#	iretl
