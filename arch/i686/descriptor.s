.set limit, 0
.set base, 0


#the IDT pointer
IDTPtr:
	.word 0
	.long 0

#the GDT pointer
GDTPtr:
	.word 0
	.long 0


#the TSS
TSS:
	.long 0x00000000	#0x00	reserved	LINK
TSS_ESP0:
	.long 0x00000000	#0x04	ESP0
TSS_SS0:
	.long 0x00000000	#0x08	reserved	SS0
	.long 0x00000000	#0x0C	ESP1
	.long 0x00000000	#0x10	reserved	SS1
	.long 0x00000000	#0x14	ESP2
	.long 0x00000000	#0x18	reserved	SS2
	.long 0x00000000	#0x1C	CR3
	.long 0x00000000	#0x20	EIP
	.long 0x00000000	#0x24	EFLAGS
	.long 0x00000000	#0x28	EAX
	.long 0x00000000	#0x2C	ECX
	.long 0x00000000	#0x30	EDX
	.long 0x00000000	#0x34	EBX
	.long 0x00000000	#0x38	ESP
	.long 0x00000000	#0x3C	EBP
	.long 0x00000000	#0x40	ESI
	.long 0x00000000	#0x44	EDI
	.long 0x00000000	#0x48	reserved	ES
	.long 0x00000000	#0x4C	reserved	CS
	.long 0x00000000	#0x50	reserved	SS
	.long 0x00000000	#0x54	reserved	DS
	.long 0x00000000	#0x58	reserved	FS
	.long 0x00000000	#0x5C	reserved	GS
	.long 0x00000000	#0x60	reserved	LDTR
TSS_IOPB:
	.long 0x00000000	#0x64	IOPB offset	.long
TSSEnd:

#registers an interrupt in the table
#two arguments, esp+8 = index, esp+12 = interrupt_address
/*.global set_interrupt_address
.type set_interrupt_address, @function
set_interrupt_address:
	#we need to conserve ebx
	push %ebx
	
	#load the parameters
	movl 8(%esp), %eax 	#index
	movl 12(%esp), %ebx #address
	
	#set ecx to the address of the entry we want to change
	movl $IDTPtr+2, %ecx
	movl (%ecx), %ecx
	lea (%ecx, %eax, 8), %ecx

	#now we get the middle and lower 16 bits of the address
	movl %ebx, %eax
	shr $16, %ebx
	#now ax contains the lower 16 bit and bx the upper 16 bit

	#fil in the data 
	movw %ax, (%ecx)
	movw %bx, 6(%ecx)	
	#movl $int_unused, %eax
	#load the old ebx
	pop %ebx
	ret*/


#loads the TSS
.global reload_tss
reload_tss:
	movw	$0x10, TSS_SS0			#first set the SS0 to the gdt datasegment (0x10, probably)
	movl	$esp0_top, TSS_ESP0		#then set the stack to our special tss_esp0 stack
	movw	$104,	TSS_IOPB+2
	movw 	$0x28, 	%ax				#set the index of the TSS in the GDT
	ltr %ax	
	ret

#loads the idt
#(limit: u16, base: u32)
.global reload_idt
reload_idt:
	movw 4(%esp), %ax
	movw %ax, IDTPtr
	movl 8(%esp), %eax
	movl %eax, IDTPtr+2
	#finally, load the table
	lidt IDTPtr
	ret

#reloads the gdt
#(limit:u16, base:u32)
.global reload_gdt
reload_gdt:
	#move the arguments to the gdt ptr
	movw 4(%esp), %ax
	movw %ax, GDTPtr
	movl 8(%esp), %eax
	movl %eax, GDTPtr+2
	#and load the gdt
	lgdt GDTPtr
	#then we reload the segments
	call reload_segments
	ret


#this routine reloads the segment registers
.global reload_segments
reload_segments:
   	lcall $0x08,$.reload_CS		##0x08 points at the new code selector
.reload_CS:
	movw	$0x10,	%ax			#0x10 points at the new data selector
	movw	%ax,	%DS
	movw	%ax,	%ES
	movw	%ax, 	%FS
	movw	%ax, 	%GS
	movw	%ax, 	%SS
.end:
	ret
