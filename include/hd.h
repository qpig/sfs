#ifndef	_ORANGES_HD_H_
#define	_ORANGES_HD_H_


#include "type.h"
#define NR_PART_PER_DRIVE 4
#define NR_SUB_PER_PART 16
#define NR_SUB_PER_DRIVE (NR_SUB_PER_PART*NR_PART_PER_DRIVE)
#define NR_PRIM_PER_DRIVE (NR_PART_PER_DRIVE + 1)
#define MAX_PRIM NR_PRIM_PER_DRIVE

#define REG_DATA	0x1F0		/*	Data				I/O		*/
#define REG_FEATURES	0x1F1		/*	Features			O		*/
#define REG_ERROR	REG_FEATURES	/*	Error				I		*/

#define REG_NSECTOR	0x1F2		/*	Sector Count			I/O		*/
#define REG_LBA_LOW	0x1F3		/*	Sector Number / LBA Bits 0-7	I/O		*/
#define REG_LBA_MID	0x1F4		/*	Cylinder Low / LBA Bits 8-15	I/O		*/
#define REG_LBA_HIGH	0x1F5		/*	Cylinder High / LBA Bits 16-23	I/O		*/
#define REG_DEVICE	0x1F6		/*	Drive | Head | LBA bits 24-27	I/O		*/

#define REG_STATUS	0x1F7		/*	Status				I		*/

#define	STATUS_BSY	0x80
#define	STATUS_DRDY	0x40
#define	STATUS_DFSE	0x20
#define	STATUS_DSC	0x10
#define	STATUS_DRQ	0x08
#define	STATUS_CORR	0x04
#define	STATUS_IDX	0x02
#define	STATUS_ERR	0x01

#define REG_CMD		REG_STATUS	/*	Command				O		*/

/*	MACRO		PORT			DESCRIPTION			INPUT/OUTPUT	*/
#define REG_DEV_CTRL	0x3F6		/*	Device Control			O		*/

#define REG_ALT_STATUS	REG_DEV_CTRL	/*	Alternate Status		I		*/

#define REG_DRV_ADDR	0x3F7		/*	Drive Address			I		*/

#define	P_PRIMARY	0
#define	P_EXTENDED	1

#define PIGS_PART	0x99	/* Orange'S partition */
#define NO_PART		0x00	/* unused entry */
#define EXT_PART	0x05	/* extended partition */

#define	NR_INODE	64	/* FIXME */
#define	NR_SUPER_BLOCK	8

#define SECTOR_SIZE		512
#define SECTOR_BITS		(SECTOR_SIZE * 8)
#define SECTOR_SIZE_SHIFT	9

#define	MINOR_hd1a		0x10
#define	MINOR_hd2a		0x20
#define	MINOR_hd2b		0x21
#define	MINOR_hd3a		0x30
#define	MINOR_hd4a		0x40
#define MINOR_BOOT		MINOR_hd2a

#define	ROOT_DEV		MAKE_DEV(DEV_HD, MINOR_BOOT)	/* 3, 0x21 */

typedef struct part_ent {
	u8 boot_ind;		
	u8 start_head;		
	u8 start_sector;	
	u8 start_cyl;		
	u8 sys_id;		
	u8 end_head;
	u8 end_sector;		
	u8 end_cyl;		
	u32 start_sect;	
	u32 nr_sects;		
} PART_ENTRY;

typedef struct part_info {
	u32 base;
	u32 size;
}PART_INFO;

typedef struct hd_info{
	int open_cnt;
	PART_INFO primary[NR_PRIM_PER_DRIVE];
	PART_INFO logical[NR_SUB_PER_DRIVE];
}HD_INFO;


struct hd_cmd {
	u8	features;
	u8	count;
	u8	lba_low;
	u8	lba_mid;
	u8	lba_high;
	u8	device;
	u8	command;
};


#define	HD_TIMEOUT		10000	/* in millisec */
#define	PARTITION_TABLE_OFFSET	0x1BE
#define ATA_IDENTIFY		0xEC
#define ATA_READ		0x20
#define ATA_WRITE		0x30
#define	DIOCTL_GET_GEO	1
/* for DEVICE register. */
#define	MAKE_DEVICE_REG(lba,drv,lba_highest) (((lba) << 6) |		\
					      ((drv) << 4) |		\
					      (lba_highest & 0xF) | 0xA0)



void hd_handler(int irq);

#endif /* _ORANGES_HD_H_ */
