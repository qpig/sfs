#include "type.h"
#include "hd.h"
#include "fs.h"

struct d_block_table_entry *get_block_table( int dev, int size, int n )
{
	int i,j=n;
	struct m_super_block *psb = get_super_block( dev );
	if( size <0 || size > 6 )
	{
		printk( "get_block_table size error %x.\n", size);
		return NULL;
	}
	for( i=0; i<NR_BLOCK_TABLE_ENTRY; i++ )
	{
		struct d_block_table_entry *bt = (psb->p_block_table)+i;
		if( bt->block_size == 1<<size && bt->inode_count != 0 )
		{
			if( j == 0 )
				return (struct d_block_table_entry *)bt;
			else
				j--;
		}
	}
	printk("can't find block_table %x.\n", size );
	return get_block_table( dev, size, n );
}
void read_block_table( struct m_super_block *psb )
{
	int i;
	struct buffer_head *bh = bread( psb->sb_dev, psb->first_block_table_sect, 1 );
	struct d_block_table_entry *pbt = (struct d_block_table_entry *)bh->pdata;
	for( i=0; i<NR_BLOCK_TABLE_ENTRY; i++ )
		psb->p_block_table[i] = pbt[i];
	brelse( bh );
}

