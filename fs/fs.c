//#include "config.h"
#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "proto.h"
#include "lib/stdio.h"
#include "string.h"

#include "hd.h"

struct dev_drv_map dd_map[] = {
	{INVALID_DRIVER},
	{INVALID_DRIVER},
	{INVALID_DRIVER},
	{TASK_HD},
	{TASK_TTY},
	{INVALID_DRIVER},
};

struct file_desc file_desc_table[NR_FILE_DESC];
struct proc *pcaller;
struct m_inode *root_inode;

static void init_fs();
static void mkfs();
static int fs_fork( MESSAGE *msg );
static int fs_exit( MESSAGE *msg );

void task_fs()
{
	printk("Task FS begins.\n");

	init_buffer( 0x300000 );
	init_fs();

	MESSAGE msg;

	while(1)
	{
		send_recv( RECEIVE, ANY, &msg );
		int src = msg.source;
		switch(msg.type)
		{
			case OPEN:
				msg.FD = do_open( &msg );
				break;
			case CLOSE:
				msg.RETVAL = do_close( &msg );
				break;
			case READ:
			case WRITE:
				msg.CNT = do_rdwr( &msg );
				break;
			case RESUME_PROC:
				src = msg.PROC_NR;
				break;
			case FORK:
				msg.RETVAL = fs_fork( &msg );
				break;
			case EXIT:
				msg.RETVAL = fs_exit( &msg );
				break;
			default:
				dump_msg("FS:unknow message:", &msg );
				assert(0);
				break;
		}

		if( msg.type != SUSPEND_PROC )
		{
			msg.type = SYSCALL_RET;
			send_recv( SEND, src, &msg );
		}
	}
	spin("FS");
}

static void init_fs()
{
	int i;
	for( i=0; i<NR_FILE_DESC; i++ )
		memset( &file_desc_table[i], 0, sizeof(struct file_desc) );
	for( i=0; i<NR_INODE; i++ )
		memset( &inode_table[i], 0, sizeof(struct m_inode ) );

	struct m_super_block *sb = super_block_table;
	for( ; sb<&super_block_table[NR_SUPER_BLOCK]; sb++ )
		sb->sb_dev = NO_DEV;

	MESSAGE driver_msg;
	driver_msg.type = DEV_OPEN;
	driver_msg.DEVICE = MINOR(ROOT_DEV);
	send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

	mkfs();

	mount_root( );
}

static void mkfs()
{
	int i;
	MESSAGE driver_msg;

	PART_INFO geo;
	driver_msg.type = DEV_IOCTL;
	driver_msg.DEVICE = MINOR(ROOT_DEV);
	driver_msg.REQUEST = DIOCTL_GET_GEO;
	driver_msg.BUF = &geo;
	driver_msg.PROC_NR = TASK_FS;
	send_recv( BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg );

	printk("dev size: 0x%x sectors\n", geo.size );

	//super block
	struct d_super_block sb;
	sb.magic = MAGIC_V1;
	sb.first_block_table_sect = FIRST_BLOCK_TABLE ;
	sb.root_inode = ROOT_INODE ;
	sb.zone_mete_sects = ZONE_METE_SECTS;
	sb.zone_data_sects = ZONE_DATA_SECTS;

	struct buffer_head bh ;
	bh.pdata = (void *)kmalloc( SECTOR_SIZE );
	bh.dev = ROOT_DEV;
	bh.blocknr = FIRST_SUPER_BLOCK;
	bh.size = 1;
	memset( bh.pdata, 0x60, SECTOR_SIZE * 2 );
	memcpy( bh.pdata, &sb, sizeof(struct d_super_block) );
	WR_SECT( &bh );

	printk("devbase: 0x%x00, sb:0x%x00, btable:0x%x00, first zone:0x%x00, "
		   "first_zone_mete: 0x%x00, first_zone_data: 0x%x00	\n",
			geo.base * 2,
			(geo.base +2) * 2,
			(geo.base +4) * 2,
			(geo.base +6) * 2,
			(geo.base +6) * 2,
			(geo.base +6 +ZONE_METE_SECTS) *2 );

	//block table
	memset( bh.pdata, 0, SECTOR_SIZE * 2 );
	struct d_block_table_entry bt;
	for( i=0; i<NR_BLOCK_TABLE_ENTRY; i++ )
	{
		bt.block_size = 1 << i;
		if( i == 0 )
		{
			bt.first_inode_num = 1;
		}
		else
		{
			struct d_block_table_entry *pbt = (struct d_block_table_entry *)bh.pdata;
			bt.first_inode_num = pbt[i-1].first_inode_num +pbt[i-1].inode_count ;
		}
		bt.free_inode_num = bt.first_inode_num ;
		bt.inode_count = ZONE_DATA_SECTS / bt.block_size;
		bt.start_imap_nr = ZONE_ALL_SECTS * i + FIRST_ZONE;
		bt.start_itable_nr = bt.start_imap_nr + ZONE_IMAP_SECTS;
		bt.start_data_nr = bt.start_imap_nr + ZONE_METE_SECTS;
		bt.dev = ROOT_DEV;
		memcpy( bh.pdata + i * sizeof(struct d_block_table_entry) , &bt, sizeof(struct d_block_table_entry) );
	}
	bh.blocknr = FIRST_BLOCK_TABLE ;
	WR_SECT( &bh );

	//imap
	memset( bh.pdata, 0, SECTOR_SIZE );
	for( i=0; i< (NR_CONSOLES +2); i++ )
		((u8 *)(bh.pdata))[0] |= 1<< i;
	assert( ((u8 *)(bh.pdata))[0] == 0x1f );
	bh.blocknr = FIRST_ZONE ;
	bh.size = 0;
	WR_SECT( &bh );
	memset( bh.pdata, 0, SECTOR_SIZE );
	((u8 *)(bh.pdata))[0] = 0xff;
	bh.blocknr = bt.start_imap_nr ;
	bh.size = 0;
	WR_SECT( &bh );

	memset( bh.pdata, 0, SECTOR_SIZE );
	for( i=1; i<bt.inode_count/SECTOR_SIZE + 1; i++ )
	{
		bh.blocknr = FIRST_ZONE + i;
		WR_SECT( &bh );
	}

	//inodes
	memset( bh.pdata, 0, SECTOR_SIZE );
	struct d_inode *pi = (struct d_inode *)bh.pdata;
	pi->mode = I_DIRECTORY;
	pi->size = 0;
	pi->start_data_sect = FIRST_ZONE + ZONE_METE_SECTS ;
	pi->next_inode_id = 0;
	pi->nlinks = 1;
	pi->bitmap = 1;
	memcpy( pi->name, "/", sizeof("/") );

	for( i=0; i<NR_CONSOLES; i++ )
	{
		pi++;
		pi->mode = I_CHAR_SPECIAL;
		pi->size = 0;
		pi->start_data_sect = MAKE_DEV( DEV_CHAR_TTY, i);
		pi->next_inode_id = 0;
		pi->nlinks = 1;
		pi->bitmap = 0;
		sprintf( pi->name, "dev_tty%d", i+1 );
	}
	bh.blocknr = FIRST_ZONE + ZONE_IMAP_SECTS ;
	WR_SECT( &bh );

	memset( bh.pdata, 0, SECTOR_SIZE );
	char *names[] ={
		"hello",
		"ls",
		"cat",
		"touch",
		"write",
		"mkdir",
		"rm"
	};
	const int cmd_cnt = 5;
	pi = (struct d_inode *)bh.pdata;
	for( i=0; i<cmd_cnt; i++ )
	{
		pi->mode = I_REGULAR;
		pi->size = 6;
		pi->start_data_sect = bt.start_data_nr + i*bt.block_size ;
		pi->next_inode_id = 0;
		pi->nlinks = 1;
		pi->bitmap = 0;
		sprintf( pi->name, names[i] );
		pi++;
	}
	bh.blocknr = bt.start_itable_nr;
	WR_SECT( &bh );
	//memset( bh.pdata, 0x99, SECTOR_SIZE );
	//bh.blocknr = bt.start_data_nr;
	//WR_SECT( &bh );

	//d_bree_node  of "/"
	memset( bh.pdata, 0, SECTOR_SIZE );
	bh.blocknr = FIRST_ZONE + ZONE_METE_SECTS ;
	struct d_name_value_pair kv_pair[] ={
		{ ".", 1 },
		{ "..", 1 },
		{ "dev_tty1", 2 },
		{ "dev_tty2", 3 },
		{ "dev_tty3", 4 },
		{ names[0], bt.first_inode_num },
		{ names[1], bt.first_inode_num + 1},
		{ names[2], bt.first_inode_num + 2},
		{ names[3], bt.first_inode_num + 3},
		{ names[4], bt.first_inode_num + 4},
		/*
		{ names[5], bt.first_inode_num + 5},
		{ names[6], bt.first_inode_num + 6},
		*/
	};
	btree_new_root( &bh, kv_pair, sizeof(kv_pair)/sizeof(kv_pair[0]) );
	WR_SECT( &bh );

	kfree( bh.pdata );
}

static int fs_fork( MESSAGE *msg )
{
	int i;
	struct proc *child = proc_table + msg->PID;
	for( i=0; i< NR_FILES; i++ )
	{
		if( child->filp[i] )
		{
			child->filp[i]->fd_count++;
			child->filp[i]->fd_inode->count++;
		}
	}
	return 0;
}

static int fs_exit( MESSAGE *msg )
{
	int i;
	struct proc *p = proc_table + msg->PID;
	for( i=0; i<NR_FILES; i++ )
	{
		if( p->filp[i] )
		{
			p->filp[i]->fd_inode->count--;
			if( --p->filp[i]->fd_count == 0 )
				p->filp[i]->fd_inode = 0;
			p->filp[i] = 0;
		}
	}
	return 0;
}
