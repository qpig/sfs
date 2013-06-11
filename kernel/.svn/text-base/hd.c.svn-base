#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "proto.h"
#include "hd.h"
#include "time.h"
#include "asm/io.h"
#include "i8259.h"
#include "errno.h"
#include "lib/stdio.h"


static void	init_hd();
static void	hd_cmd_out(struct hd_cmd* cmd);
static int	waitfor(int mask, int val, int timeout);
static void	interrupt_wait();
static void hd_identify(int drive);
static void	print_identify_info	(u16* hdinfo);
extern void port_read( int reg, void *buf, int size );
extern void port_write( u16 port, void *buf, int size );
static void hd_open( int device );
static void partition( int device, int opt );
static void print_hdinfo( HD_INFO *hd_if );
static void hd_close( int device );
static void hd_rdwt( MESSAGE *p );
static void hd_ioctl( MESSAGE *p);

static	u8	hd_status;
static	u8	hdbuf[SECTOR_SIZE * 2];
static HD_INFO hd_info[1];

void task_hd()
{
	printk("task_hd begin\n");
	MESSAGE msg;

	init_hd();

	while (1) {
		send_recv(RECEIVE, ANY, &msg);

		int src = msg.source;

		switch (msg.type) {
		case DEV_OPEN:
			hd_open(msg.DEVICE);
			break;
		case DEV_CLOSE:
			hd_close(msg.DEVICE);
			break;
		case DEV_READ:
		case DEV_WRITE:
			hd_rdwt(&msg);
			break;
		case DEV_IOCTL:
			hd_ioctl(&msg);
			break;
		default:
			dump_msg("HD driver::unknown msg", &msg);
			spin("FS::main_loop (invalid msg.type)");
			break;
		}

		send_recv(SEND, src, &msg);
	}
}

static void init_hd()
{
	/* Get the number of drives from the BIOS data area */
	u8 * pNrDrives = (u8*)(0x475);
	printk("NrDrives:%d.\n", *pNrDrives);
	assert(*pNrDrives);

	put_irq_handler(AT_WINI_IRQ, hd_handler);
	enable_irq(CASCADE_IRQ);
	enable_irq(AT_WINI_IRQ);

	int i;
	for( i=0; i< sizeof(hd_info)/ sizeof(hd_info[0]); i++ )
		memset( &hd_info[i], 0, sizeof(hd_info[0]) );
	hd_info[0].open_cnt = 0;
}

static void hd_close( int device )
{
	int drive = 0;
	hd_info[drive].open_cnt-- ;
}

static void hd_rdwt( MESSAGE *p )
{
	int drive = 0;
	u32 sect_nr = (u32)p->POSITION;
	int logidx = ( p->DEVICE- MINOR_hd1a) % NR_SUB_PER_DRIVE;
	sect_nr += p->DEVICE < MAX_PRIM ?
		hd_info[drive].primary[p->DEVICE].base :
		hd_info[drive].logical[logidx].base ;

	struct hd_cmd cmd;
	cmd.features = 0;
	cmd.count = p->CNT ;
	cmd.lba_low = sect_nr & 0xff;
	cmd.lba_mid = (sect_nr >> 8) & 0xff;
	cmd.lba_high = (sect_nr >> 16) & 0xff;
	cmd.device = MAKE_DEVICE_REG(1, drive, (sect_nr >> 24)& 0xff);
	cmd.command = (p->type == DEV_READ) ? ATA_READ : ATA_WRITE ;
	hd_cmd_out(&cmd);

	int block_left = p->CNT;
	void *la = (void *)va2la( p->PROC_NR, p->BUF );
	void *hd_la = (void *)va2la( TASK_HD, hdbuf );

	while(block_left)
	{
		if( p->type == DEV_READ )
		{
			interrupt_wait();
			port_read( REG_DATA, hdbuf, SECTOR_SIZE );
			memcpy( la, hd_la, SECTOR_SIZE );
		}
		else
		{
			if( !waitfor( STATUS_DRQ, STATUS_DRQ, HD_TIMEOUT) )
				panic("hd wirting error.");
			port_write( REG_DATA, la, SECTOR_SIZE );
			interrupt_wait();
		}
		block_left -= 1;
		la += SECTOR_SIZE ;
	}
}

static void hd_ioctl( MESSAGE *p)
{
	int drive = 0;
	int device = p->DEVICE;
	HD_INFO *hd_if = &hd_info[drive];

	if( p->REQUEST == DIOCTL_GET_GEO )
	{
		void *dst = va2la( p->PROC_NR, p->BUF );
		void *src = va2la( TASK_HD,
				device < MAX_PRIM ?
				&hd_if->primary[device] :
				&hd_if->logical[device-MINOR_hd1a]);
		memcpy( dst, src, sizeof(PART_INFO) );
	}
	else
		assert(0);
}

static void hd_open( int device )
{
	int drive = 0;
	hd_identify(drive);
	if( hd_info[drive].open_cnt ++ == 0 )
	{
		partition( drive, P_PRIMARY );
		print_hdinfo( &hd_info[drive] );
	}
}

static void get_part_table( int drive, int sect_nr, struct part_ent *entry )
{
	struct hd_cmd  cmd;
	cmd.features = 0;
	cmd.count = 1 ;
	cmd.lba_low = sect_nr & 0xff;
	cmd.lba_mid = (sect_nr >> 8) & 0xff;
	cmd.lba_high = (sect_nr >> 16) & 0xff;
	cmd.device = MAKE_DEVICE_REG( 1, drive, (sect_nr >> 24) & 0xff );
	cmd.command = ATA_READ;
	hd_cmd_out( &cmd );
	interrupt_wait();

	port_read( REG_DATA, hdbuf, SECTOR_SIZE );
	memcpy( entry, hdbuf + PARTITION_TABLE_OFFSET, sizeof(PART_ENTRY) * NR_PART_PER_DRIVE );
}

static void partition( int device, int opt )
{
	int i;
	int drive = 0;
	HD_INFO *hd_if =  &hd_info[drive];
	PART_ENTRY part_table[NR_PART_PER_DRIVE];

	if( opt == P_PRIMARY )
	{
		get_part_table( drive, 0, part_table );
		for( i=0; i< NR_PART_PER_DRIVE; i++ )
		{
			if( part_table[i].sys_id == NO_PART )
				continue;
			hd_if->primary[i+1].base = part_table[i].start_sect;
			hd_if->primary[i+1].size = part_table[i].nr_sects;
			if( part_table[i].sys_id == EXT_PART )
				partition( drive+i+1, P_EXTENDED );
		}
	}
	else if( opt == P_EXTENDED )
	{
		int j = device ;
		int ext_start_sect= hd_if->primary[j].base;
		int s = ext_start_sect;
		int nr_1st_sub = (j-1) * NR_SUB_PER_PART ;

		for( i=0; i< NR_SUB_PER_PART; i++ )
		{
			int dev_nr = nr_1st_sub + i;
			get_part_table( drive, s, part_table );
			hd_if->logical[dev_nr].base = s+part_table[0].start_sect;
			hd_if->logical[dev_nr].size = part_table[0].nr_sects;
			s = ext_start_sect + part_table[1].start_sect;
			if( part_table[1].sys_id == NO_PART )
				break;
		}
	}
	else
		assert(0);
}

static void print_hdinfo( HD_INFO *hd_if )
{
	int i;
	for( i=0; i<NR_PART_PER_DRIVE+1; i++ )
	{
		printk("%sPART_%d: base %d(0x%x), size %d(0x%x) (int sector)\n",
				i == 0 ? " " : "   ",
				i,
				hd_if->primary[i].base,
				hd_if->primary[i].base,
				hd_if->primary[i].size,
				hd_if->primary[i].size);
	}
	for( i=0; i<NR_SUB_PER_DRIVE;  i++ )
	{
		if( hd_if->logical[i].size == 0 )
			continue;
		printk("      %sPART_%d: base %d(0x%x), size %d(0x%x) (int sector)\n",
				i == 0 ? " " : "   ",
				i,
				hd_if->logical[i].base,
				hd_if->logical[i].base,
				hd_if->logical[i].size,
				hd_if->logical[i].size);
	}
}

static void hd_identify(int drive)
{
	struct hd_cmd cmd;
	cmd.device  = MAKE_DEVICE_REG(0, drive, 0);
	cmd.command = ATA_IDENTIFY;
	hd_cmd_out(&cmd);
	interrupt_wait();
	port_read(REG_DATA, hdbuf, SECTOR_SIZE);

	print_identify_info((u16*)hdbuf);

	u16 *hd_if = (u16 *) hdbuf;
	hd_info[drive].primary[0].base = 0;
	hd_info[drive].primary[0].size = ( (int)hd_if[61] << 16 ) + hd_if[60] ;
}

static void print_identify_info(u16* hdinfo)
{
	int i, k;
	char s[64];

	struct iden_info_ascii {
		int idx;
		int len;
		char * desc;
	} iinfo[] = {{10, 20, "HD SN"}, /* Serial number in ASCII */
		     {27, 40, "HD Model"} /* Model number in ASCII */ };

	for (k = 0; k < sizeof(iinfo)/sizeof(iinfo[0]); k++) {
		char * p = (char*)&hdinfo[iinfo[k].idx];
		for (i = 0; i < iinfo[k].len/2; i++) {
			s[i*2+1] = *p++;
			s[i*2] = *p++;
		}
		s[i*2] = 0;
		printk("%s: %s\n", iinfo[k].desc, s);
	}

	int capabilities = hdinfo[49];
	printk("LBA supported: %s\n",
	       (capabilities & 0x0200) ? "Yes" : "No");

	int cmd_set_supported = hdinfo[83];
	printk("LBA48 supported: %s\n",
	       (cmd_set_supported & 0x0400) ? "Yes" : "No");

	int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
	printk("HD size: %dMB\n", sectors * 512 / 1000000);
}

static void hd_cmd_out(struct hd_cmd* cmd)
{
	if (!waitfor(STATUS_BSY, 0, HD_TIMEOUT))
		panic("hd error.");

	/* Activate the Interrupt Enable (nIEN) bit */
	outb_p( 0, REG_DEV_CTRL );
	/* Load required parameters in the Command Block Registers */
	outb_p( cmd->features, REG_FEATURES );
	outb_p( cmd->count, REG_NSECTOR );
	outb_p( cmd->lba_low, REG_LBA_LOW );
	outb_p( cmd->lba_mid, REG_LBA_MID );
	outb_p( cmd->lba_high, REG_LBA_HIGH );
	outb_p( cmd->device, REG_DEVICE );
	/* Write the command code to the Command Register */
	outb_p( cmd->command, REG_CMD );
}

static void interrupt_wait()
{
	MESSAGE msg;
	send_recv(RECEIVE, INTERRUPT, &msg);
}

static int waitfor(int mask, int val, int timeout)
{
	int t = get_ticks();

	while(((get_ticks() - t) * 1000 / HZ) < timeout)
		if ((in_byte(REG_STATUS) & mask) == val)
			return 1;

	return 0;
}

void hd_handler(int irq)
{
	hd_status = in_byte(REG_STATUS);

	inform_int(TASK_HD);
}
