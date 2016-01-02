.set VIRTUAL_OFFSET, 0x00000000

.global page_directory
.global page_directory_end
.global page_table
.global page_table_end

.align 0x1000
.lcomm page_directory, 1024
page_directory_end:

.align 0x1000
.lcomm page_table, 1024*1024
page_table_end:


.global pagingInitialize
.type pagingInitialize, @function
pagingInitialize:
    #loop to set each entry in the page directory to a page table
    mov $0, %eax
    mov $page_directory, %ecx
  .set_directory_tables:
    lea -VIRTUAL_OFFSET(%ecx,%eax,4), %ebx #calculate the address of the page directory
    or %ebx, 0x00000003
    mov %ebx, -VIRTUAL_OFFSET(%ecx, %eax, 4) #set the value in the table
    add $1, %eax
    cmp $1024, %eax
    jne .set_directory_tables
    ret

.global pagingUsePage
.type pagingUsePage,@function
pagingUsePage: #int page_index, ptr physicaladdr
    #movl 4(%esp), %eax #get the arguments
    #movl 8(%esp), %ebx #get the arguments
    leal page_table - VIRTUAL_OFFSET(%eax), %eax

.global pagingEnable
.type pagingEnable, @function
pagingEnable:
    #cr3 needs to be a pointer to the page table
    mov $page_directory - VIRTUAL_OFFSET, %eax
    mov %eax, %cr3

    #the page bit of cr0 needs to be set
    mov %cr0, %eax
    or %eax, 0x80000000
    mov %eax, %cr0
    ret
