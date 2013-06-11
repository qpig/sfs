#include "type.h"
#include "hd.h"
#include "fs.h"
#include "proc.h"
#include "stddef.h"
#include "string.h"

struct m_super_block super_block_table[NR_SUPER_BLOCK];
extern void read_block_table( struct m_super_block *psb );
static void read_super_block(int dev)
{
	int i;
	for( i=0; i<NR_SUPER_BLOCK; i++)
		if( super_block_table[i].sb_dev == dev )
			return ;

	struct buffer_head *bh =  bread( dev, FIRST_SUPER_BLOCK, 1 );

	for( i=0; i<NR_SUPER_BLOCK; i++ )
		if( super_block_table[i].sb_dev == NO_DEV )
			break;
	if( i == NR_SUPER_BLOCK )
		panic("super_block slots used up" );
	struct m_super_block *psb =(struct m_super_block *)bh->pdata;
	super_block_table[i] = *psb;
	super_block_table[i].sb_dev = dev;
	read_block_table( super_block_table + i );
	brelse( bh );
}

struct m_super_block *get_super_block( int dev )
{
	struct m_super_block *sb = super_block_table;
	for( ; sb<super_block_table+NR_SUPER_BLOCK; sb++ )
		if( sb->sb_dev == dev )
			return sb;
	panic("super block of devie %d not found.\n", dev );
	return NULL;
}

void free_super_block( int dev )
{
	struct m_super_block *psb;

	if( dev == ROOT_DEV )
	{
		printk("root diskette change!\n");
	}
	psb = get_super_block( dev );
	if( psb != NULL )
	{
		psb->sb_dev = NO_DEV;
	}
}

void mount_root( void )
{
	int i;
	struct m_super_block *p;
	
	if( 64 != sizeof( struct d_inode ))
		panic(" bad inode size!\n");
	for( i=0; i< NR_FILE_DESC; i++ )
		file_desc_table[i].fd_count = 0;
	for( p=super_block_table; p<super_block_table+NR_SUPER_BLOCK; p++ )
	{
		memset( p, 0, sizeof(struct m_super_block) );
	}
	read_super_block( ROOT_DEV );
	if( !( p = get_super_block(ROOT_DEV)) )
		panic("unable to mount root!\n");
	read_block_table( p );
}
