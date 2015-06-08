.set limit, 0
.set base, 0


#the gdtr
GDTPtr:
	.word GDT - GDTEnd
	.long GDT
GDT:
	.long 0x00000000, 0x00000000	# 00 NULL Entry
	.long 0x0000FFFF, 0x00CF9A00	# 08 PL0 Code
	.long 0x0000FFFF, 0x00CF9200	# 10 PL0 Data
	.long 0x0000FFFF, 0x00CFFA00	# 18 PL3 Code
	.long 0x0000FFFF, 0x00CFF200	# 20 PL3 Data
	.long 0x00000068, 0x00408900 	# 28 TSS
GDTEnd:


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
.global reload_tss
reload_tss:
	enter	$0,		$0
	movw	$0x10,	TSS_SS0			#first set the SS0 to the gdt datasegment (0x10, probably)
	movl	$esp0_top, TSS_ESP0		#then set the stack to our special tss_esp0 stack
	movw	$104,	TSS_IOPB+2
	movw 	$0x28, 	%ax				#set the index of the TSS in the GDT
	ltr %ax	

	leave
	ret

#this routine reloads the segment
.global reload_segments
reload_segments:
	enter 	$0,		$0

	movl	$TSS,	%eax			#set up the TSS base
	movw	%ax,	GDT+0x28+2
	shr		$16,	%eax
	movb	%al,	GDT+0x28+4
	movb	%ah,	GDT+0x28+7

	lgdt GDTPtr					#load the gdt_pointer in the gdt register
   	lcall $0x08,$.reload_CS		##0x08 points at the new code selector
.reload_CS:
	movw   $0x10, %ax			#0x10 points at the new data selector
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
