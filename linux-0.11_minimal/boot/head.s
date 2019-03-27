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
	call _setup_idt
	call _setup_gdt
	movl $0x10,%eax		# reload all the segment registers
	mov %ax,%ds			# after changing gdt. CS was already
	mov %ax,%es			# reloaded in 'setup_gdt'
	mov %ax,%fs
	mov %ax,%gs
	lss _stack_start,%esp
	xorl %eax,%eax
1:	incl %eax			# check that A20 really IS enabled
	movl %eax,0x000000	# loop forever if it isn`t
	cmpl %eax,0x100000
	je 1b

	movl %cr0,%eax		# check math chip
	andl $0x80000011,%eax	# Save PG,PE,ET

	orl $2,%eax			# set MP
	movl %eax,%cr0
	jmp after_page_tables

_setup_idt:
	lea ignore_int,%edx
	movl $0x00080000,%eax
	movw %dx,%ax		/* selector = 0x0008 = cs */
	movw $0x8E00,%dx	/* interrupt gate - dpl=0, present */

	lea _idt,%edi
	mov $256,%ecx
rp_sidt:
	movl %eax,(%edi)
	movl %edx,4(%edi)
	addl $8,%edi
	dec %ecx
	jne rp_sidt
	lidt idt_descr
	ret

_setup_gdt:
	lgdt gdt_descr
	ret

.org 0x1000
pg0:

.org 0x2000
pg1:

.org 0x3000
pg2:

.org 0x4000
pg3:

.org 0x5000
after_page_tables:
	pushl $0		# These are the parameters to main :-)
	pushl $0
	pushl $0
	pushl $L6		# return address for main, if it decides to.
	pushl $_start_kernel
	jmp setup_paging
L6:
	jmp L6			# main should never return here, but just in case, we know what happens.

int_msg:
	.asciz "Unknown interrupt\n\r"

.align 2
ignore_int:
	pushl %eax
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	pushl $int_msg
	call _printk
	popl %eax
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret

.align 2
setup_paging:
	movl $1024*5,%ecx		/* 5 pages - pg_dir+4 page tables */
	xorl %eax,%eax
	xorl %edi,%edi			/* pg_dir is at 0x000 */
	cld;rep;stosl
	movl $pg0+7,_pg_dir		/* set present bit/user r/w */
	movl $pg1+7,_pg_dir+4	/*  ---------     --------- */
	movl $pg2+7,_pg_dir+8	/*  ---------     --------- */
	movl $pg3+7,_pg_dir+12	/*  ---------     --------- */
	movl $pg3+4092,%edi
	movl $0xfff007,%eax		/*  16Mb - 4096 + 7 (r/w user,p) */
	std
1:	stosl					/* fill pages backwards - more efficient :-) */
	subl $0x1000,%eax
	jge 1b
	xorl %eax,%eax			/* pg_dir is at 0x0000 */
	movl %eax,%cr3			/* cr3 - page directory start */
	movl %cr0,%eax
	orl $0x80000000,%eax
	movl %eax,%cr0			/* set paging (PG) bit */
	ret						/* this also flushes prefetch-queue */

.align 2
.word 0
idt_descr:
	.word 256*8-1			# idt contains 256 entries
	.long _idt

.align 2
.word 0
gdt_descr:
	.word 256*8-1			# so does gdt (not that that`s any
	.long _gdt				# magic number, but it works for me :^)

.align 8
_idt:
	.fill 256,8,0			# idt is uninitialized

_gdt:
	.quad 0x0000000000000000/* NULL descriptor */
	.quad 0x00c09a0000000fff/* 16Mb */
	.quad 0x00c0920000000fff/* 16Mb */
	.quad 0x0000000000000000/* TEMPORARY - don`t use */
	.fill 252,8,0			/* space for LDT``s and TSS`s etc */
