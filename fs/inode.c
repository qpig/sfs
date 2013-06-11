#include "type.h"
#include "string.h"
#include "fs.h"

struct m_inode inode_table[NR_INODE];
static int set_imap_first_zero( struct d_block_table_entry *bt, struct m_inode *inode );
static int clr_imap_bit( struct d_block_table_entry *bt, struct m_inode *inode );
static struct d_block_table_entry *get_block_table_from_nr( int dev, int nr );
static void get_inode_info_from_nr( struct m_inode *inode, int dev, int nr );
//static int get_itable_sect_nr( int dev, int nr );
//static int get_imap_sect_nr( int dev, int nr );
static void free_memery_inode( struct m_inode *inode );
static struct m_inode *alloc_memery_inode( );
static struct d_inode *get_inode_itable( int dev, int nr );
static void put_inode_itable( struct m_inode *inode );
static void free_inode( struct m_inode *inode );
static int put_inode_data( struct m_inode *inode );

void invalidate_inode( int dev )
{
	int i;
	struct m_inode *pinode;
	struct buffer_head *bh ;
	pinode = inode_table;
	for( i=0; i<NR_INODE; i++,pinode++ )
	{
		if( pinode->dev == dev )
		{
			if( pinode->count )
				printk("inode used on removed disk!\n");
			pinode->dev = pinode->dirt = 0;
			bh = bread( pinode->dev, pinode->start_data_sect, pinode->size );
			bwrite( bh );
			brelse( bh );
		}
	}
}
static void free_inode( struct m_inode *inode )
{
	free_memery_inode( inode );
}
struct m_inode *get_inode( int dev, int nr )
{
	struct m_inode *inode = NULL;
	int i;
	if( nr == 0 )
		return NULL;
	for( i=0; i<NR_INODE; i++ )
	{
		if( inode_table[i].dev == dev && inode_table[i].inode_num == nr )
			inode = inode_table + i;
	}
	if( inode == NULL )
	{
		inode = alloc_memery_inode();
		get_inode_info_from_nr( inode, dev, nr );
		*((struct d_inode *)inode) = *get_inode_itable( dev, nr );
	}
	return inode;
}
void put_inode( struct m_inode *inode )
{
	put_inode_itable( inode );
	put_inode_data( inode );
	free_memery_inode( inode );
}

void mov_inode_data( struct m_inode *des, struct m_inode *src )
{
	struct buffer_head *d_bh = bread( des->dev, des->start_data_sect, des->size );
	struct buffer_head *s_bh = bread( src->dev, src->start_data_sect, src->size );
	memcpy( d_bh->pdata, s_bh->pdata, SECTOR_SIZE * (1<<src->size) );
	memset( s_bh->pdata, 0, SECTOR_SIZE * (1<< src->size) );
	bwrite( d_bh );
	bwrite( s_bh );
}

struct buffer_head *get_inode_data( int dev, int nr )
{
	struct m_inode inode;
	get_inode_info_from_nr( &inode, dev, nr );
	struct buffer_head *bh = bread( inode.dev, inode.start_data_sect, inode.size );
	return bh;
}

static int put_inode_data( struct m_inode *inode )
{
	struct buffer_head *bh = get_inode_data( inode->dev, inode->inode_num );
	return bwrite( bh );
}

struct m_inode *new_inode( int dev, int size )
{
	struct m_super_block *sb;
	struct d_block_table_entry *bt;
	struct m_inode *inode = NULL;
	if( (sb=get_super_block(dev)) == NULL )
		panic("new_inode with unknow device!\n");
	if( (bt=get_block_table(dev, size, 0)) == NULL )
		panic("new_inode not find block table!\n");
	inode = alloc_memery_inode();
	set_imap_first_zero( bt, inode );
	return inode;
}

int delete_inode( struct m_inode *inode )
{
	struct d_block_table_entry *bt;
	if( (bt=get_block_table(inode->dev, inode->size, 0)) == NULL )
		panic("delete_inode not find block table!\n");
	clr_imap_bit( bt, inode );
	inode->mode = 0;
	inode->size = 0;
	inode->start_data_sect = 0;
	inode->next_inode_id = 0;
	inode->nlinks = 0;
	inode->bitmap = 0;
	put_inode_itable( inode );
	free_memery_inode( inode );
	return 1;
}

static struct m_inode *alloc_memery_inode( )
{
	int i;
	struct m_inode *inode = NULL;
	for( i=0; i<NR_INODE; i++ )
	{
		//if( inode_table[i].dirt == 1 )
		//	put_inode( &inode_table[i] );
		if( inode_table[i].count == 0)
		{
			inode = inode_table + i ;
			inode->count = 1;
			return inode;
		}
	}
	if( inode == NULL )
		printk("inode is used up!\n");
	else
	{
		memset( inode, 0, sizeof( struct m_inode) );
		return inode;
	}
	return NULL;
}
static void free_memery_inode( struct m_inode *inode )
{
	if( inode->count > 0 )
		inode->count--;
	//if( inode->count == 0 )
	//	memset( inode, 0, sizeof(struct m_inode) );
}
static struct d_inode *get_inode_itable( int dev, int nr )
{
	struct m_inode inode;
	struct buffer_head *bh;
	struct d_inode *tmp;
	get_inode_info_from_nr( &inode, dev, nr );
	bh = bread( inode.dev, inode.start_itable_sect, 0 );
	tmp = (struct d_inode *)(bh->pdata);
	brelse( bh );
	return tmp + (nr - inode.zone_first_inode_num)%8 ;
}
static void put_inode_itable( struct m_inode *inode )
{
	struct buffer_head *bh = bread( inode->dev, inode->start_itable_sect, 0 );
	struct d_inode *tmp = (struct d_inode *)(bh->pdata );
	tmp = tmp + (inode->start_itable_sect - inode->zone_first_inode_num)%8;
	*tmp = *((struct d_inode *)inode);
	free_inode( inode );
	bwrite( bh );
}


static struct d_block_table_entry *get_block_table_from_nr( int dev, int nr )
{
	struct m_super_block *sb;
	struct d_block_table_entry *bt;
	int i;
	if( (sb=get_super_block(dev)) == NULL )
		panic("new_inode with unknow device!\n");
	bt = sb->p_block_table;
	for( i=0; i<NR_BLOCK_TABLE_ENTRY; i++,bt++ )
	{
		if( bt->first_inode_num <= nr && nr < bt->first_inode_num + bt->inode_count )
			return bt;
	}
	return NULL;
}

static void get_inode_info_from_nr( struct m_inode *inode, int dev, int nr )
{
	struct d_block_table_entry *bt = get_block_table_from_nr( dev, nr );
	int i=0,size = bt->block_size;
	while( size != 1 )
	{
		size = size >> 1;
		i++;
	}
	inode->size = i;
	inode->start_imap_sect = bt->start_imap_nr + (nr - bt->first_inode_num) / SECTOR_SIZE;
	inode->start_itable_sect = bt->start_itable_nr + (nr - bt->first_inode_num) /16;
	inode->start_data_sect = bt->start_data_nr + (nr - bt->first_inode_num) *bt->block_size;
	inode->zone_first_inode_num = bt->first_inode_num;
	inode->inode_num = nr;
	inode->dev = dev;
}
/*
static int get_imap_sect_nr( int dev, int nr)
{
	struct d_block_table_entry *bt = get_block_table_from_nr( dev, nr );
	return bt->start_imap_nr + (nr - bt->first_inode_num) /SECTOR_SIZE;
}
static int get_itable_sect_nr( int dev, int nr)
{
	struct d_block_table_entry *bt = get_block_table_from_nr( dev, nr );
	return bt->start_imap_nr + nr/16; 
}
*/

static int clr_imap_bit( struct d_block_table_entry *bt, struct m_inode *inode )
{
	int imap_nr = bt->start_imap_nr;
	int off = inode->inode_num - bt->first_inode_num;
	if( inode->inode_num < bt->free_inode_num )
		bt->free_inode_num = inode->inode_num;
	int block = off / SECTOR_SIZE;
	struct buffer_head *bh = bread( bt->dev, imap_nr + block, 0 );
	int i = off / 32;
	int j = off % 32;
	u32 *tmp = (u32 *)(bh->pdata);
	tmp = tmp + i;
	*tmp = *tmp & ~(1<<j);
	bwrite( bh );
	return 1;
}
static int set_imap_first_zero( struct d_block_table_entry *bt, struct m_inode *inode )
{
	int imap_nr = bt->start_imap_nr ;
	int free_block = (bt->free_inode_num - bt->first_inode_num )/SECTOR_SIZE ;
	int i,j,inode_nr;
	struct buffer_head *bh ;

	for( ; free_block < bt->inode_count/SECTOR_SIZE +1; free_block++ )
	{
		bh = bread( bt->dev, imap_nr + free_block, 0 );
		u32 *tmp = (u32 *)(bh->pdata);
		for( i=0; i<SECTOR_SIZE/32; i++,tmp++ )
		{
			if( *tmp != 0xffffffff )
			{
				for( j=0; j<32; j++ )
					if( (~(*tmp) & (1<<j)) != 0 )
						break;
				inode_nr = bt->free_inode_num + free_block *512 + i * 32 + j;
				if( j > bt->free_inode_num + bt->inode_count )
				{
					printk("inode is used up!\n");
					return 0;
				}

				*tmp = *tmp | 1 << j;
				bwrite( bh );

				bt->free_inode_num = inode_nr + 1;
				inode->dev = bt->dev;
				inode->size = bt->block_size;
				inode->inode_num = inode_nr;
				inode->start_data_sect = bt->start_data_nr + (inode_nr - bt->first_inode_num) * bt->block_size;
				inode->start_itable_sect = bt->start_itable_nr + (inode_nr - bt->first_inode_num)/8;
				inode->start_imap_sect = bt->start_imap_nr + (inode_nr - bt->first_inode_num)/SECTOR_SIZE;
				inode->zone_first_inode_num = bt->first_inode_num;
				inode->next_inode_id = 0;
				inode->nlinks = 1;
				inode->update = 1;
				inode->dirt = 0;
				return inode_nr;
			}
		}
		brelse( bh );
	}
	return 0;
}
/*
void sync_inodes( void )
{
	int i;
	struct m_inode *pinode;
	pinode = inode_table;
	for( i=0; i< NR_INODE; i++,pinode++ )
	{
		if( pinode->dirt )
			write_inode(pinode);	
	}
}

static void read_inode( struct m_inode *pinode )
{
	struct m_super_block *psb;
	struct m_
}
struct inode *get_inode( int dev, int num )
{
	if( num == 0 )
		return 0;
	struct inode *p, *q = 0 ;

	for( p=&inode_table[0]; p<&inode_table[NR_INODE]; i++ )
	{
		if( p->i_cnt )
		{
			if( (p->i_dev == dev) && (p->i_num == num) )
			{
				p->i_cnt++;
				return p;
			}
		}
		else
		{
			q->i_dev = dev;
			q->i_num = num;
			q->i_cnt = 1;
			break;
		}
	}

	struct super_block *sb = get_su
}
*/
