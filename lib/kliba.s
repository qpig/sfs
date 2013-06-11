.data
.extern disp_pos

.text
.global	disp_str, out_byte ,in_byte,disp_color_str,disp_al,enable_irq,disable_irq,disable_int,enable_int
.global port_read, port_write

disable_int:
	cli
	ret
enable_int:
	sti
	ret

disp_al:
	push	%ecx
	push	%edx
	push	%edi

	mov		(disp_pos),%edi
	mov		$0x0f,%ah
	mov		%al,%dl
	shr		$4,%al
	mov		$2,%ecx
.begin:	
	and		$0b01111,%al
	cmp		$9,%al
	ja		b1
	add		$'0',%al
	jmp		b2
b1:	
	sub		$0x0a,%al
	add		$'A',%al
b2:
	mov		%ax,%gs:(%edi)
	add		$2,%edi
	mov		%dl,%al
	loop	.begin

	mov		%edi,(disp_pos)
	
	pop		%edi
	pop		%edx
	pop		%ecx
	ret

disp_int:
	mov		4(%esp),%eax
	shr		$24,%eax
	call	disp_al

	mov		4(%esp),%eax
	shr		$16,%eax
	call	disp_al

	mov		4(%esp),%eax
	shr		$8,%eax
	call	disp_al

	mov		4(%esp),%eax
	call	disp_al

	mov		$0x07,%ah
	mov		$'h',%al
	push	%edi
	mov		(disp_pos),%edi
	mov		%ax,%gs:(%edi)
	add		$4,%edi
	mov		%edi,(disp_pos)
	pop		%edi

	ret



disp_str:
	push	%ebp
	mov		%esp,%ebp

	mov		8(%ebp),%esi
	mov		(disp_pos),%edi
	mov		$0x0f,%ah
.ds1:
	lodsb
	test	%al,%al
	jz		.ds2
	cmp		$0xa,%al
	jnz		.ds3
	push	%eax
	mov		%edi,%eax
	mov		$160,%bl
	div		%bl
	and		$0xff,%eax
	inc		%eax
	mov		$160,%bl
	mul		%bl
	mov		%eax,%edi
	pop		%eax
	jmp		.ds1
.ds3:
	mov		%ax,%gs:(%edi)
	add		$2,%edi
	jmp		.ds1
.ds2:
	mov		%edi,(disp_pos)

	pop		%ebp
	ret

disp_color_str:
	push	%ebp
	mov		%esp,%ebp

	mov		8(%ebp),%esi
	mov		(disp_pos),%edi
	mov		12(%ebp),%ah
.dc1:
	lodsb 
	test	%al,%al
	jz		.dc2
	cmp		$0x0a,%al
	jnz		.dc3
	push	%eax
	mov		%edi,%eax
	mov		$160,%bl
	div		%bl
	and		$0x0ff,%eax
	inc		%eax
	mov		$160,%bl
	mul		%bl
	mov		%eax,%edi
	pop		%eax
	jmp		.dc1
.dc3:
	mov		%ax,%gs:(%edi)
	add		$2,%edi
	jmp		.dc1
.dc2:	
	mov		%edi,(disp_pos)

	pop		%ebp
	ret

/*void out_byte(u16 port,u8 value)*/
out_byte:
	mov		4(%esp),%edx
	mov		8(%esp),%al
	out		%al,%dx
	nop
	nop
	ret

/*u8 in_byte(u16 port) */
in_byte:
	mov		4(%esp),%edx
	xor		%eax,%eax
	in		%dx,%al
	nop
	nop
	ret


disable_irq:
	mov		4(%esp),%ecx
	pushf
	cli
	mov		$1,%ah
	rol		%cl,%ah
	cmp		$8,%cl
	jae		disable_8
disable_0:
	in		$0x21,%al
	test	%ah,%al
	jnz		dis_already
	or		%ah,%al
	out		%al,$0x21
	popf
	mov		$1,%eax
	ret
disable_8:
	in		$0xa1,%al
	test	%ah,%al
	jnz		dis_already
	or		%ah,%al
	out		%al,$0xa1
	popf
	mov		$1,%eax
	ret
dis_already:	
	popf	
	xor		%eax,%eax
	ret


enable_irq:
	mov		4(%esp),%ecx
	pushf
	cli
	mov		$~1,%ah
	rol		%cl,%ah
	cmp		$8,%cl
	jae		enable_8
enable_0:
	in		$0x21,%al
	and		%ah,%al
	out		%al,$0x21
	popf
	ret
enable_8:
	in		$0xa1,%al
	and		%ah,%al
	out		%al,$0xa1
	popf
	ret
	

port_read:
	mov 4(%esp), %edx
	mov 8(%esp), %edi
	mov 12(%esp), %ecx
	shr	$1, %ecx
	cld
	rep	insw
	ret

port_write:
	mov 4(%esp), %edx
	mov 8(%esp), %esi
	mov 12(%esp), %ecx
	shr	$1, %ecx
	cld
	rep	outsw
	ret
