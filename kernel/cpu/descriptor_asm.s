.set limit, 0
.set base, 0


#the IDT pointer
.global IDTPtr
IDTPtr:
	.word 0
	.long 0

#the GDT pointer
.global GDTPtr
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

#loads the TSS
#(datasegment: u16, gdtindex: u16)
.global initialize_tss
initialize_tss:
	lea 4(%esp), %eax
	movw	%ax, TSS_SS0 - 0xC0000000			#first set the SS0 to the gdt datasegment (0x10, probably)
	movl	$esp0_top - 0xC0000000, TSS_ESP0		#then set the stack to our special tss_esp0 stack
	movw	$104,	TSS_IOPB+2
	movw 	8(%esp), 	%ax				#set the index of the TSS in the GDT
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
.global reloadGdt
reloadGdt:
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
   	ljmp $0x08,$.reload_CS		##0x08 points at the new code selector
.reload_CS:
	movw	$0x10,	%ax			#0x10 points at the new data selector
	movw	%ax,	%DS
	movw	%ax,	%ES
	movw	%ax, 	%FS
	movw	%ax, 	%GS
	movw	%ax, 	%SS
.end:
	ret
