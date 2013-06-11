DispAL:
	push	%ecx
	push	%edx
	push	%edi

	mov		(dwDispPos),%edi
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

	mov		%edi,(dwDispPos)
	
	pop		%edi
	pop		%edx
	pop		%ecx
	ret


DispInt:
	mov		4(%esp),%eax
	shr		$24,%eax
	call	DispAL

	mov		4(%esp),%eax
	shr		$16,%eax
	call	DispAL

	mov		4(%esp),%eax
	shr		$8,%eax
	call	DispAL

	mov		4(%esp),%eax
	call	DispAL

	mov		$0x07,%ah
	mov		$'h',%al
	push	%edi
	mov		(dwDispPos),%edi
	mov		%ax,%gs:(%edi)
	add		$4,%edi
	mov		%edi,(dwDispPos)
	pop		%edi

	ret


DispStr:
	push	%ebp
	mov		%esp,%ebp
	push	%ebx
	push	%esi
	push	%edi

	mov		8(%ebp),%esi
	mov		(dwDispPos),%edi
	mov		$0x0f,%ah
D1:
	lodsb	
	test	%al,%al
	jz		D2
	cmp		$0x0a,%al
	jnz		D3
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
	jmp		D1
D3:
	mov		%ax,%gs:(%edi)
	add		$2,%edi
	jmp		D1

D2:		
	mov		%edi,(dwDispPos)
	pop		%edi
	pop		%esi
	pop		%ebx
	pop		%ebp
	ret

DispReturn:	
	push	$szReturn
	call	DispStr
	add		$4,%esp
	ret
/*void MemCopy(void *es:pDest,void *ds:pSrc,int iSize)*/
MemCopy:
	push	%ebp
	mov		%esp,%ebp

	push	%esi
	push	%edi
	push	%ecx

	mov		8(%ebp),%edi
	mov		12(%ebp),%esi
	mov		16(%ebp),%ecx
.M1:
	cmp		$0,%ecx
	jz		.M2
	
	mov		%ds:(%esi),%al
	inc		%esi

	mov		%al,%es:(%edi)
	inc		%edi

	dec		%ecx
	jmp		.M1
.M2:

	mov		8(%ebp),%eax

	pop		%ecx
	pop		%edi
	pop		%esi

	mov		%ebp,%esp
	pop		%ebp

	ret
	
