.text
.globl _idt,_gdt,_pg_dir
_pg_dir:
.globl startup_32
startup_32:
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	lss _stack_start,%esp
	pushl $0		# These are the parameters to main :-)
	pushl $0
	pushl $0
	pushl $0		# return address for main, if it decides to.
	pushl $_start_kernel
    ret
