#include "type.h"
#include "const.h"
#include "stddef.h"
#include "fs.h"


static u32 km_map[K_MEMORY_SECTS/32] = { 0 };
static void* p_free = (void *)0x700000;

static void set_map( void *p );
static void clear_map( void *p );
//static u8 get_map( void *p );
static void *find_next_free( void *p );

void *kmalloc( int size )
{
	int blocks = (size -1)/SECTOR_SIZE + 1 ;
	if( p_free >= (void *)0x800000 )
		return NULL;
	void *p = p_free;
	assert(p>=(void *)0x700000);
	if( p < (void *)0x800000 )
	{
		set_map(p);
		p_free = find_next_free( p );
		return p;
	}
	else
	{
		p_free = (void *)0x800000;
		printk("K_MEMORY is used up\n");
		return NULL;
	}
}

void kfree( void* p )
{
	if( p < p_free )
		p_free = p;
	clear_map( p );
}

static void set_map( void *p )
{
	u32 i = ((u32)p-K_MEMORY_BEGIN) / SECTOR_SIZE / 32;
	u32 j = ((u32)p-K_MEMORY_BEGIN) / SECTOR_SIZE %32;
	km_map[i] |= 1 << j;
}
static void clear_map( void *p )
{
	u32 i = ((u32)p-K_MEMORY_BEGIN) / SECTOR_SIZE / 32;
	u32 j = ((u32)p-K_MEMORY_BEGIN) / SECTOR_SIZE %32;
	km_map[i] &= ~(1 << j);
}
/*
static u8 get_map( void *p )
{
	u32 i = ((u32)p-K_MEMORY_BEGIN) / SECTOR_SIZE / 32;
	u32 j = ((u32)p-K_MEMORY_BEGIN) / SECTOR_SIZE %32;
	if( km_map[i] & (1<<j) )
		return TRUE;
	else
		return FALSE;
}
*/

static void *find_next_free( void *p )
{
	u32 i = ((u32)p-K_MEMORY_BEGIN) / SECTOR_SIZE / 32;
	u32 j;
	while( km_map[i] == 0xffffffff )
	{
		i++;
	}
	assert( km_map[i] != 0xffffffff )
	for( j=0; j<32; j++ )
		if( km_map[i] & (1<<j) )
			;
		else
			break;
	return (void *)((i*32+j)*SECTOR_SIZE + K_MEMORY_BEGIN ) ;
}
