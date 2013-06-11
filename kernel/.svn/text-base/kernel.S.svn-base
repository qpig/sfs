
.extern gdt_ptr,idt_ptr,p_proc_ready,tss,disp_pos,k_reenter
/*rogram*/
.extern	cstart,kernel_main,exception_handler,spurious_irq,clock_handler,sys_call_table


.bss
StackSpace:		.fill	2048
StackTop:

.text

#include "sconst.h"
.global _start ,restart,sys_call

.global divide_error,single_step_exception,nmi,breakpoint_exception
.global overflow,bounds_check,inval_opcode,copr_not_available
.global double_fault,copr_seg_overrun,inval_tss,segment_not_present
.global stack_exception,general_protection,page_fault,copr_error
.global  hwint00
.global  hwint01
.global  hwint02
.global  hwint03
.global  hwint04
.global  hwint05
.global  hwint06
.global  hwint07
.global  hwint08
.global  hwint09
.global  hwint10
.global  hwint11
.global  hwint12
.global  hwint13
.global  hwint14
.global  hwint15


_start:
	mov		$StackTop,%esp
	movl	$0,(disp_pos)	
	sgdt	(gdt_ptr)
	call	cstart
	lgdt	(gdt_ptr)
	lidt	(idt_ptr)

	jmp		$SELECTOR_KERNEL_CS,$csinit
csinit:
	movb	$0x0f,%ah
	movb	$'K',%al
	mov		%ax,%gs:((80*1+39)*2)
	xor		%eax,%eax
	mov		$SELECTOR_TSS,%ax
	ltr		%ax
	jmp		kernel_main

.macro	hwint_master	n
	call	save
	in		$0x21,%al
	or		$(1<<\n),%al
	out		%al,$0x21

	mov		$EOI,%al
	out		%al,$0x20
	sti

	push	$(\n)
	mov		$irq_table,%ebx
	call	*(4*\n)(%ebx)
	add		$4,%esp
	cli
	
	in		$0x21,%al
	and		$~(1<<\n),%al
	out		%al,$0x21
/*out kernel*/
	ret
.endm

.macro hwint_slave		n
	call	save
	in		$0xA1,%al
	or		$(1 << (\n - 8)),%al
	out		%al,$0xA1

	mov		$EOI,%al
	out		%al,$0x20
	nop
	out		%al,$0xA0
	sti

	push	$(\n)
	mov		$irq_table,%ebx
	call	*(4*\n)(%ebx)
	add		$4,%esp
	cli

	in		$0xA1,%al
	and		$~(1 << (\n - 8)),%al
	out		%al,$0xA1
	ret
.endm


.align 16
sys_call:
	call	save
	pushl	(p_proc_ready)
	sti
	push	%esi
	push	%edx
	push	%ecx
	push	%ebx
	call	*sys_call_table(,%eax,4)
	add		$16,%esp
	pop     %esi
	mov		%eax,(EAXREG-P_STACKBASE)(%esi)
	cli
	ret

hwint00:    
	hwint_master 0
save:
	pushal	
	push	%ds
	push	%es
	push	%fs
	push	%gs
	mov		%edx,%esi
	mov		%ss,%dx
	mov		%dx,%ds
	mov		%dx,%es
	mov		%dx,%fs
	mov		%esi,%edx

	mov		%esp,%esi

	incl	(k_reenter)
	cmpl	$0,(k_reenter)
	jne		.re_enter1
/*in kernel*/
	mov		$StackTop,%esp
	push	$restart
	jmp		*(RETADR-P_STACKBASE)(%esi)
.re_enter1:
	push	$restart_reenter
	jmp		*(RETADR-P_STACKBASE)(%esi)
	

.align 16
hwint01:                # Interrupt routine for irq 1 (keyboard)
		hwint_master	1

.align 16
hwint02:                #Interrupt routine for irq 2 (cascade!)
        hwint_master    2

.align   16
hwint03:                # Interrupt routine for irq 3 (second serial)
        hwint_master    3

.align   16
hwint04:                # Interrupt routine for irq 4 (first serial)
        hwint_master    4

.align   16
hwint05:                # Interrupt routine for irq 5 (XT winchester)
        hwint_master    5

.align   16
hwint06:                # Interrupt routine for irq 6 (floppy)
        hwint_master    6

.align   16
hwint07:                # Interrupt routine for irq 7 (printer)
        hwint_master    7


.align   16
hwint08:                # Interrupt routine for irq 8 (realtime clock).
        hwint_slave     8

.align   16
hwint09:                # Interrupt routine for irq 9 (irq 2 redirected)
        hwint_slave     9

.align   16
hwint10:                # Interrupt routine for irq 10
        hwint_slave     10

.align   16
hwint11:                # Interrupt routine for irq 11
        hwint_slave     11

.align   16
hwint12:                # Interrupt routine for irq 12
        hwint_slave     12

.align   16
hwint13:                # Interrupt routine for irq 13 (FPU exception)
        hwint_slave     13

.align   16
hwint14:                # Interrupt routine for irq 14 (AT winchester)
        hwint_slave     14

.align   16
hwint15:                # Interrupt routine for irq 15
        hwint_slave     15

divide_error:
	push	$0xffffffff
	push	$0
	jmp		exception
single_step_exception:
	push	$0xffffffff
	push	$1
	jmp		exception
nmi:
	push	$0xffffffff
	push	$2
	jmp		exception
breakpoint_exception:
	push	$0xffffffff
	push	$3
	jmp		exception
overflow:
	push	$0xffffffff
	push	$4
	jmp		exception
bounds_check:
	push	$0xffffffff
	push	$5
	jmp		exception
inval_opcode:
	push	$0xffffffff
	push	$6
	jmp		exception
copr_not_available:
	push	$0xffffffff
	push	$7
	jmp		exception
double_fault:
	push	$8
	jmp		exception
copr_seg_overrun:
	push	$0xffffffff
	push	$9
	jmp		exception
inval_tss:
	push	$10
	jmp		exception
segment_not_present:
	push	$11
	jmp		exception
stack_exception:
	push	$12
	jmp		exception
general_protection:
	push	$13
	jmp		exception
page_fault:
	push	$14
	jmp		exception
copr_error:
	push	0xffffffff
	push	$16
	jmp		exception

exception:
	call	exception_handler
	add		$8,%esp
	jmp		.

restart:
	mov		(p_proc_ready),%esp
	lldt	(P_LDT_SEL)(%esp)
	lea		(P_STACKTOP)(%esp),%eax
	mov		$tss,%ebx
	mov		%eax,(TSS3_S_SP0)(%ebx)
	
restart_reenter:
	decl	(k_reenter)
	pop		%gs
	pop		%fs
	pop		%es
	pop		%ds
	popal

	add		$4,%esp

	iret
