#include "type.h"
#include "hd.h"
#include "fs.h"
#include "stddef.h"
#include "proc.h"
#include "const.h"

#define NR_HASH 307
#define NR_BUFFER    7

#define _hashfn(dev,block,size) ( (((u32)(dev^block))%NR_HASH) + (size)*NR_HASH )
#define hash(bh)    hash_table[_hashfn((bh)->dev,(bh)->blocknr,(bh)->size)]

struct buffer_head *start_buffer[NR_BUFFER] ;
int buffer_size[NR_BUFFER];
void * buffer_start = (void *)K_MEMORY_BEGIN ;
struct buffer_head *hash_table[NR_HASH*NR_BUFFER];
static struct buffer_head *free_list[NR_BUFFER];
int buffer_num[NR_BUFFER]  = { 0 };
int sync_dev( int dev );

void init_buffer( u32 buffer_end )
{
	init_hash();//for btree
	int i;
	void *b = (void *)buffer_end;
	struct buffer_head *h;
	for( i=0; i<NR_BUFFER; i++ )
	{
		start_buffer[i] = buffer_start + i*0x20000;
		buffer_size[i] = 1 << i;
	}
	if( buffer_end <= 1<<20 )
		panic("buffer end less than buffer start 1M!\n");
	
	for( i=NR_BUFFER-1; i>=0; i-- )
	{
		h = start_buffer[i];
		while( (b -= 512*buffer_size[i]) >= (void *)(h+1) )
		{
			h->pdata = (char *)b;
			h->blocknr = 0;
			h->dev = 0;
			h->size = i;
			h->dirt = 1;
			h->count = 0;
			h->wait = NULL;
			h->prev = NULL;
			h->next = NULL;
			h->prev_free = h-1;
			h->next_free = h+1;
			h++;
			buffer_num[i]++;
		}
		h--;
		free_list[i] = start_buffer[i];
		free_list[i]->prev_free =  h;
		h->next_free = free_list[i];
		b = start_buffer[i];
	}
	for( i=0; i<NR_HASH*NR_BUFFER; i++ )
		hash_table[i] = NULL;
}

static inline void remove_from_queues( struct buffer_head *bh )
{
	//remove form hash
	if( bh->next )
		bh->next->prev = bh->prev;
	if( bh->prev )
		bh->prev->next = bh->next;
	if( hash(bh) == bh )
		hash(bh) = bh->next;
	if( !(bh->prev_free) || !(bh->next_free) )
		panic("free block list error!\n");
	//remove from freelist
	bh->prev_free->next_free = bh->next_free;
	bh->next_free->prev_free = bh->prev_free;
	if( free_list[bh->size] == bh )
		free_list[bh->size] = bh->next_free ;
}

static inline void insert_into_queues( struct buffer_head *bh )
{
	//insert into freelist
	bh->next_free = free_list[bh->size];
	bh->prev_free = free_list[bh->size]->prev_free;
	free_list[bh->size]->prev_free->next_free = bh;
	free_list[bh->size]->prev_free = bh;

	//insert into hash	
	bh->next = NULL;
	bh->prev = NULL;
	if( !bh->dev )
		return ;
	bh->next = hash( bh );
	hash(bh) = bh;
	bh->next->prev = bh;
}

static struct buffer_head *find_buffer( int dev, int block, int size )
{
	struct buffer_head *tmp,bh;
	bh.dev = dev;
	bh.blocknr = block;
	bh.size = size;
	for( tmp=hash(&bh); tmp!=NULL; tmp = tmp->next )
	{
		if( tmp->dev == (u16)dev && tmp->blocknr == (u64)block && tmp->size == (u8)size )
			return tmp;
	}
	return NULL;
}
struct buffer_head *get_from_hash_table( int dev, int block, int size )
{
	struct buffer_head *bh;
	if( !(bh=find_buffer( dev, block, size)) )
		return NULL;
	bh->count++;
	if( bh->dev == dev && bh->blocknr == block && bh->size == size )
		return bh;
	return NULL;
}

struct buffer_head *getblk( int dev, int block, int size )
{
	struct buffer_head  *tmp,*bh;
	if( (bh = get_from_hash_table( dev, block, size)) != NULL )
	{
		if( bh->dirt != 0 )
		{
			bwrite( bh );
			bh->dirt = 0;
		}
		return bh;
	}
	tmp = free_list[size];
	do{
		if( tmp->count )
			continue;
		bh = tmp;
		break;
	}while( (tmp=tmp->next_free) != free_list[size] );
	if( !bh )
		panic("can't find free block, size %d.\n", 1<<size);
	bh->count = 1;
	bh->dirt = 1;
	remove_from_queues(bh);
	bh->dev = dev;
	bh->blocknr = block;
	insert_into_queues(bh);
	return bh;
}

void brelse( struct buffer_head *bh )
{
	if( bh == NULL )
		return ;

	if( bh->count == 0 )
	{
		assert("trying to free free buffer!\n");
	}
	else
		bh->count--;
	if( bh->dirt != 0 )
	{
		WR_SECT( bh );
		bh->dirt = 0;
	}
}
void bdirty( struct buffer_head *bh )
{
	if( !bh )
		return ;
	bh->dirt = 1;
}

struct buffer_head *bread( int dev, int block, int size )
{
	struct buffer_head *bh;
	if( !(bh=getblk(dev,block,size)) )
		panic("bread:getblk return NULL !\n");
	if( bh->dirt != 0 )
	{
		RD_SECT( bh );
		bh->dirt = 0;
	}
	return bh;
}

int bwrite( struct buffer_head *bh )
{
	bdirty( bh );
	brelse( bh );
	return 0;
}

int sys_sync(void)
{
	int i,j;
	struct buffer_head *bh;
	
	for( i=0; i<NR_BUFFER; i++ )
	{
		bh = start_buffer[i];
		for( j=0; j<buffer_num[i]; j++, bh++ )
			if( bh->dirt )
				WR_SECT(bh);
	}
	return 0;
}

int sync_dev( int dev )
{
	int i,j;
	struct buffer_head *bh;

	for( i=0; i<NR_BUFFER; i++ )
	{
		bh = start_buffer[i];
		for( j=0; j<buffer_num[i]; j++, bh++ )
		{
			if( bh->dev != dev )
				continue;
			if( bh->dev == dev && bh->dirt )
				WR_SECT(bh);
		}
	}
	return 0;
}


