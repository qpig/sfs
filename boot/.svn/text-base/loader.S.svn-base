.code16
.text
    jmp LABEL_BEGIN     /* jump over the .data section. */
#include "fat12hdr.h"
#include "pm.h"
#include "loader.h"

LABEL_GDT:		Descriptor	0, 0, 0
LABEL_DESC_FLAT_C:	Descriptor	0, 0xfffff,	(DA_CR + DA_32 + DA_LIMIT_4K)
LABEL_DESC_FLAT_RW:	Descriptor	0, 0xfffff, (DA_DRW + DA_32 + DA_LIMIT_4K)
LABEL_DESC_VIDEO:	Descriptor	0xb8000,	0xffff,(DA_DRW + DA_DPL3)

.set		GdtLen,	.-LABEL_GDT
GdtPtr:		.2byte	GdtLen-1
			.4byte	BaseOfLoader*0x10+LABEL_GDT

.set	SelectFlatC,	(LABEL_DESC_FLAT_C	- LABEL_GDT)
.set	SelectFlatRW,	(LABEL_DESC_FLAT_RW	- LABEL_GDT)
.set	SelectVideo,	(LABEL_DESC_VIDEO	- LABEL_GDT+SA_RPL3)


/* Program starts here. */
LABEL_BEGIN:
    mov     %cs, %ax    /* Move code segment address(CS) to data segment */
    mov     %ax, %ds    /* register(DS), ES and SS. Because we have      */
    mov     %ax, %es    /* embedded .data section into .code section in  */
    mov     %ax, %ss    /* the start(mentioned in the NOTE above).        */
    mov     $0x100, %sp
	movb	$0,%dh
	call	DispStrRealMode

/*get number of memery*/
	mov		$0,%ebx
	mov		$_MemChkBuf,%di
MemChkLoop:
	mov		$0xe820,%eax
	mov		$20,%ecx
	mov		$0x534d4150,%edx
	int		$0x15
	jc		MemChkFail
	add		$20,%di
	incl	(_dwMCRNumber)
	cmp		$0,%ebx
	jne		MemChkLoop
	jmp		MemChkOK
MemChkFail:
	movl	$0,(_dwMCRNumber)
MemChkOK:



/*find KERNEL.BIN*/
	movw	$SecNoOfRootDir,(wSectorNo)
/*reset driver 0*/	
	xorb	%ah,%ah
	xorb	%dl,%dl
	int		$0x13

LABEL_SEARCH_IN_ROOT_DIR_BEGIN:

	cmpw	$0,(wRootDirSizeForLoop)
	jz		LABEL_NO_KERNELBIN
	decw	(wRootDirSizeForLoop)
	mov		$BaseOfKernelFile,%ax
	mov		%ax,%es
	mov		$OffsetOfKernelFile,%bx
	mov		(wSectorNo),%ax
	movb	$1,%cl
	call	ReadSector	
	mov		$KernelFileName,%si
	mov		$OffsetOfKernelFile,%di
	cld
	mov		$0x10,%dx
LABEL_SEARCH_FOR_KERNELBIN:

	cmp		$0,%dx
	jz		LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR
	dec		%dx
	mov		$11,%cx
LABEL_CMP_FILENAME:
	cmp		$0,%cx
	jz		LABEL_FILENAME_FOUND
	dec		%cx
	lodsb
	cmp		%es:(%di),%al
	jz		LABEL_GO_ON
	jmp		LABEL_DIFFERENT
LABEL_GO_ON:
	inc		%di
	jmp		LABEL_CMP_FILENAME
LABEL_DIFFERENT:
	and		$0xffe0,%di
	add		$0x20,%di
	mov		$KernelFileName,%si
	jmp		LABEL_SEARCH_FOR_KERNELBIN
LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	addw	$1,(wSectorNo)
	jmp		LABEL_SEARCH_IN_ROOT_DIR_BEGIN
LABEL_NO_KERNELBIN:
	mov		$2,%dh
	call	DispStrRealMode
	jmp		.
LABEL_FILENAME_FOUND:
	mov		$RootDirSectors,%ax
	and		$0xffe0,%di
	add		$0x1a,%di
	mov		%es:(%di),%cx
	push	%cx
	add		%ax,%cx
	add		$DeltaSecNo,%cx
	mov		$BaseOfKernelFile,%ax
	mov		%ax,%es
	mov		$OffsetOfKernelFile,%bx
	mov		%cx,%ax
LABEL_GOON_LOADING_FILE:
	push	%ax
	push	%bx
	movb	$0x0e,%ah
	movb	$'.',%al
	movb	$0x0f,%bl
	int		$0x10
	pop		%bx
	pop		%ax

	mov		$1,%cl
	call	ReadSector
	pop		%ax
	call	GetFATEntry
	cmp		$0x0fff,%ax
	jz		LABEL_FILE_LOADED
	push	%ax
	mov		$RootDirSectors,%dx
	add		%dx,%ax
	add		$DeltaSecNo,%ax
	add		(BPB_BytsPerSec),%bx
	jc		.z1
	jmp		.z2
.z1:
	push	%ax
	mov		%es,%ax
	add		$0x1000,%ax
	mov		%ax,%es
	pop		%ax
.z2:
	jmp		LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:
	call	KillMotor
	movb	$1,%dh
	call	DispStrRealMode

LABEL_JMP_PROTECT:

	lgdtw	GdtPtr
	cli
	mov		$2,%dh
	in		$0x92,%al
	or		$0x02,%al
	out		%al,$0x92
	
	mov		%cr0,%eax
	or		$1,%eax
	mov		%eax,%cr0
	mov		$2,%dh
	
	ljmpl	$SelectFlatC, $LABEL_PM_START +0x90000
	jmp		.



wSectorNo:	.2byte	0
wRootDirSizeForLoop:	.2byte	RootDirSectors
bOdd:		.byte	0
dwKernelSize:	.4byte	0

KernelFileName:	.asciz	"KERNEL  BIN"
.set	MessageLength,	9
LoadMessage:	.ascii	"Loading  "
Message1:		.ascii	"Ready.   "
Message2:		.ascii	"NoKERNEL "


DispStrRealMode:
	mov		$MessageLength,%ax
	mulb	%dh
	add		$LoadMessage,%ax
	mov		%ax,%bp
	mov		%ds,%ax
	mov		%ax,%es
	mov		$MessageLength,%cx
	mov		$0x1301,%ax
	mov		$0x7,%bx
	movb	$0x0,%dl
	addb	$3,%dh
	int		$0x10
	ret

ReadSector:
	push	%ebp
	mov		%esp,%ebp
	sub		$2,%esp

	movb	%cl,-2(%ebp)
	push	%bx
	movb	(BPB_SecPerTrk),%bl
	div		%bl
	incb	%ah
	mov		%ah,%cl
	mov		%al,%dh
	shrb	$1,%al
	movb	%al,%ch
	andb	$1,%dh
	pop		%bx
	
	movb	(BS_DrvNum),%dl
GoOnReading:
	movb	$2,%ah
	movb	-2(%ebp),%al
	int		$0x13
	jc		GoOnReading
	add		$2,%esp
	pop		%ebp
	ret	

GetFATEntry:
	push	%es
	push	%bx
	push	%ax
	mov		$BaseOfKernelFile,%ax
	sub		$0x100,%ax
	mov		%ax,%es
	pop		%ax
	mov		$3,%bx
	mul		%bx
	mov		$2,%bx
	div		%bx
	movb	%dl,(bOdd)
LABEL_EVEN:
	xor		%dx,%dx
	mov		(BPB_BytsPerSec),%bx
	div		%bx

	push	%dx
	mov		$0,%bx
	add		$SecNoOfFAT1,%ax
	mov		$2,%cl
	call	ReadSector
	pop		%dx
	add		%dx,%bx
	mov		%es:(%bx),%ax
	cmpb	$0,(bOdd)
	jz		LABEL_EVEN_2
	shr		$4,%ax
LABEL_EVEN_2:
	and		$0xfff,%ax
LABEL_GET_FAT_ENTRY_OK:
	pop		%bx
	pop		%es
	ret
KillMotor:
	push	%dx
	mov		$0x3f2,%dx
	mov		$0,%al
	out		%al,%dx
	pop		%dx
	ret

.code32
.align 32
LABEL_PM_START:
	mov		$SelectVideo,%ax
	mov		%ax,%gs
	
	mov		$SelectFlatRW,%ax
	mov		%ax,%ds
	mov		%ax,%es
	mov		%ax,%fs
	mov		%ax,%ss
	mov		$TopOfStack,%esp
	
	pushl	$szMemChkTitle
	call	DispStr
	add		$4,%esp
	
	call	DispMemInfo 
	call	SetupPaging
	
	mov		$0x0f,%ah
	mov		$'P',%al
	mov		%al,%gs:((80 *0 +39)*2)
	call	InitKernel
	
	jmp		$SelectFlatC , $KernelEntryPointPhyAddr
	movl	$BootParamMagic, (BootParamAddr)
	mov		(dwMemSize), %eax
	mov		%eax, (BootParamAddr + 4)
	mov		$BaseOfKernelFile, %eax
	shl		$4, %eax
	add		$OffsetOfKernelFile, %eax
	mov		%eax, (BootParamAddr + 8)

	jmp		.

PrintF:
	mov		$0x0F,%ah
	mov		$'F',%al
	mov		%al,%gs:((80 *2+39)*2)
	ret
#include	"lib.h"
DispMemInfo:
	push	%esi
	push	%edi
	push	%ecx

	mov		$MemChkBuf,%esi
	mov		(dwMCRNumber),%ecx
.loop:
	mov		$5,%edx
	mov		$ARDStruct,%edi
.l1:
	pushl	(%esi)
	call	DispInt
	pop		%eax
	stosl	
	add		$4,%esi
	dec		%edx
	cmp		$0,%edx
	jnz		.l1
	call	DispReturn
	cmpl	$1,(dwType)
	jne		.l2
	mov		(dwBaseAddrLow),%eax
	add		(dwLengthLow),%eax
	cmp		(dwMemSize),%eax
	jb		.l2
	mov		%eax,(dwMemSize)
.l2:
	loop	.loop
	
	call	DispReturn
	push	$szRAMSize
	call	DispStr
	add		$4,%esp

	pushl	(dwMemSize)
	call	DispInt
	add		$4,%esp

	pop		%ecx
	pop		%edi
	pop		%esi
	ret

SetupPaging:
	xor		%edx,%edx
	mov		(dwMemSize),%eax
	mov		$0x400000,%ebx
	div		%ebx
	mov		%eax,%ecx
	test	%edx,%edx
	jz		.no_remainder
	inc		%ecx
.no_remainder:
	push	%ecx
	
	mov		$SelectFlatRW,%ax
	mov		%ax,%es
	mov		$PageDirBase,%edi
	xor		%eax,%eax
	mov		$(PageTblBase | PG_P | PG_USU | PG_RWW),%eax
.S1:
	stosl
	add		$4096,%eax
	loop	.S1
	
	pop		%eax
	mov		$1024,%ebx
	mul		%ebx
	mov		%eax,%ecx
	mov		$PageTblBase,%edi
	xor		%eax,%eax
	mov		$(PG_P | PG_USU | PG_RWW),%eax
.S2:
	stosl
	add		$4096,%eax
	loop	.S2
	
	mov		$PageDirBase,%eax
	mov		%eax,%cr3
	mov		%cr0,%eax
	or		$0x80000000,%eax
	mov		%eax,%cr0
	ret


InitKernel:
	xor		%esi,%esi
	mov		$(BaseOfKernelFilePhyAddr)+0x2c,%ecx
	
	mov		(%ecx),%cx
	movzx	%cx,%ecx
	
	mov		$(BaseOfKernelFilePhyAddr)+ 0x1c,%esi
	mov		(%esi),%esi
	add		$(BaseOfKernelFilePhyAddr),%esi

.Begin:
	mov		(%esi),%eax
	cmp		$0,%eax
	jz		.NoAction
	pushl	0x10(%esi)
	mov		0x4(%esi),%eax
	add		$BaseOfKernelFilePhyAddr,%eax
	push	%eax
	
	pushl	0x08(%esi)
	call	MemCopy

	add		$12,%esp
.NoAction:	
	add		$0x20,%esi
	dec		%ecx
	jnz		.Begin
	
	ret

.data
.align 32
LABEL_DATA:
_szMemChkTitle:	.asciz	"BaseAddrL  BaseAddrH  LengthLow  LengthHigh  Type\n"
_szRAMSize:		.asciz	"RAM size:"
_szReturn:		.byte	0x0a,0

_dwMCRNumber:	.4byte	0
_dwDispPos:		.4byte	(80 *6 ) *2
_dwMemSize:		.4byte	0
_ARDStruct:
	_dwBaseAddrLow:		.4byte	0
	_dwBaseAddrHigh:	.4byte	0
	_dwLengthLow:		.4byte	0
	_dwLengthHigh:		.4byte	0
	_dwType:			.byte	0
_MemChkBuf:		.fill	256


/*protect use*/
.set	szMemChkTitle,	BaseOfLoaderPhyAddr	+	_szMemChkTitle
.set	szRAMSize,		BaseOfLoaderPhyAddr	+	_szRAMSize
.set	szReturn,		BaseOfLoaderPhyAddr	+	_szReturn
.set	dwDispPos,		BaseOfLoaderPhyAddr	+	_dwDispPos
.set	dwMemSize,		BaseOfLoaderPhyAddr	+	_dwMemSize
.set	dwMCRNumber,	BaseOfLoaderPhyAddr	+	_dwMCRNumber
.set	ARDStruct,		BaseOfLoaderPhyAddr	+	_ARDStruct
	.set	dwBaseAddrLow,	BaseOfLoaderPhyAddr	+	_dwBaseAddrLow
	.set	dwBaseAddrHigh,	BaseOfLoaderPhyAddr	+	_dwBaseAddrHigh
	.set	dwLengthLow,	BaseOfLoaderPhyAddr	+	_dwLengthLow
	.set	dwLengthHigh,	BaseOfLoaderPhyAddr	+	_dwLengthHigh
	.set	dwType,			BaseOfLoaderPhyAddr	+	_dwType
.set	MemChkBuf,		BaseOfLoaderPhyAddr	+	_MemChkBuf

StackSpace:	.fill	1024
.set TopOfStack,	BaseOfLoader*0x10+ .
