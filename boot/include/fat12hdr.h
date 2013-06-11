BS_OEMName:		.ascii "VERY PIG"

BPB_BytsPerSec:		.2byte 512
BPB_SecPerCluster:	.byte	1
BPB_ResvdSecCnt:	.2byte	1
BPB_NumFATs:		.byte	2
BPB_RootEntCnt:		.2byte	224
BPB_TotSec16:		.2byte	2880
BPB_Media:			.byte	0xf0
BPB_FATSz16:		.2byte	9
BPB_SecPerTrk:		.2byte	18
BPB_NumHeads:		.2byte	2
BPB_HiddSec:		.4byte	0
BPB_TotSec32:		.4byte	0

BS_DrvNum:			.byte	0
BS_Reserved1:		.byte	0
BS_BootSig:			.byte	0x29
BS_VolID:			.4byte	0
BS_VolLab:			.ascii	"mypig  0.01"
BS_FileSysType:		.ascii	"FAT12"


.set	FATs,	9
.set	RootDirSectors,	14
.set	SecNoOfRootDir,	19
.set	SecNoOfFAT1,	1
.set	DeltaSecNo,		17
