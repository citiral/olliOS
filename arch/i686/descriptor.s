.set limit, 0
.set base, 0

#data where the pointer will be stored
.global gdt_pointer
gdt_pointer:
	.word limit
	.long base



GDTPtr:
	.word GDT - GDTEnd
	.long GDT
GDT:
	.long 0x00000000, 0x00000000	/* 00 NULL Entry */
	.long 0x0000FFFF, 0x00CF9A00	/* 08 PL0 Code */
	.long 0x0000FFFF, 0x00CF9200	/* 10 PL0 Data */
	.long 0x0000FFFF, 0x00CFFA00	/* 18 PL3 Code */
	.long 0x0000FFFF, 0x00CFF200	/* 20 PL3 Data */
GDTEnd:

#this routine reloads the segment
.global reload_segments
reload_segments:
	#stack handling
	push %ebp
	mov %esp, %ebp

	#get the stack variables
	#mov 48(%esp), %eax
	#mov %eax, gdt_pointer+2
	#mov 8(%esp), %ax
	#mov %ax, gdt_pointer
	#leave
	#ret

	lgdt GDTPtr					#load the gdt_pointer in the gdt register
	#jmp .end									#Reload CS register containing code selector:
	#mov $0x08, %eax
   	lcall $0x08,$.reload_CS			##0x08 points at the new code selector

.reload_CS:
	movw   $0x10, %ax					#0x10 points at the new data selector
	movw   %ax, %DS
	movw   %ax, %ES
	movw   %ax, %FS
	movw   %ax, %GS
	movw   %ax, %SS

.end:
	leave
	ret


	
#	pushal
#	push $95
#	push $30
#	call callback_test
#	popal
