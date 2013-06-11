#include "type.h"
#include "string.h"
#include "fs.h"

#define MAX_ENTRY 63
#define HASH_TYPE 1

u32 crypt_table[0x500];
static int do_btree_insert( struct m_inode *inode, struct d_btree_node *p_bnode, int level, u32 key, u32 inode_num );
static struct d_btree_node *btree_init( struct buffer_head *bh );
static int get_inode_empty_sector( struct m_inode *inode );
static int  do_btree_split_root( struct m_inode *inode, struct d_btree_node *p_bnode );
static int do_btree_split( struct m_inode *inode, struct d_btree_node *p_bnode);
static int btree_find_pos( struct d_btree_node *p_bnode, u32 key );
static int insert_pair( struct d_btree_node *p_bnode, u32 key, u32 value );
static int do_btree_find( struct m_inode *inode, struct d_btree_node *p_bnode, int level, int key );
static int do_btree_remove( struct m_inode *inode, struct d_btree_node *p_node, int level, int key );
void init_hash()
{
	u32 seed = 0x00100001, index1 = 0, index2 = 0, i;
	for( index1=0; index1<0x100; index1++ )
	{
		for( index2=index1,i=0; i<5; i++,index2+=0x100 )
		{
			u32 tmp1,tmp2;
			seed = (seed*125+3) %0x2aaaab;
			tmp1 = (seed&0xffff) << 0x10;
			seed = (seed*125+3) %0x2aaaab;
			tmp2 = (seed&0xffff);
			crypt_table[index2] = ( tmp1 | tmp2 );
		}
	}
}

u32 hash_string( char *name, u8 hash_type )
{
	u32 seed1 = 0x7fed7fed, seed2 = 0xeeeeeeee;
	while( *name != 0 )
	{
		seed1 = crypt_table[(hash_type<<8) +*name] ^ (seed1 +seed2);
		seed2 = *name +seed1 + seed2 + (seed2 << 5) +3;
		name++ ;
	}
	return seed1;
}

struct d_btree_node *btree_new_root( struct buffer_head *bh, struct d_name_value_pair *pkv, int num )
{
	char tmp[20] = {0};
	int i;
	struct d_btree_node *bt_root = btree_init( bh );
	memset( bh->pdata, 0, sizeof(struct d_btree_node) *(1<<bh->size) );

	bt_root->num = 0;
	bt_root->isroot = 1;
	bt_root->level = 0;
	bt_root->prev_node_index = 0;
	bt_root->next_node_index = 0;
	for( i=0; i<num; i++ )
	{
		memcpy( tmp, pkv[i].name, 19 );
		if( insert_pair( bt_root, hash_string( tmp, HASH_TYPE ), pkv[i].value ) == -1 )
			return NULL;
	}
	return bt_root;
}

static struct d_btree_node *btree_init( struct buffer_head *bh )
{
	struct d_btree_node *p_btree = (struct d_btree_node *)(bh->pdata);
	return p_btree;
}

int btree_find( struct m_inode *inode, char *name )
{
	int ret = -1;
	char tmp[20] = {0};
	memcpy( tmp, name, 19 );
	u32 key = hash_string( tmp, HASH_TYPE );
	struct buffer_head *bh = get_inode_data( inode->dev, inode->inode_num );
	ret = do_btree_find( inode, NULL, 0,  key );
	brelse( bh );
	return ret;
}
static int do_btree_find( struct m_inode *inode, struct d_btree_node *p_bnode, int level, int key )
{
	struct buffer_head *bh = get_inode_data( inode->dev, inode->inode_num );
	struct d_btree_node *bt_root = btree_init( bh );
	if( p_bnode == NULL )
		p_bnode = bt_root;
	if( p_bnode->level > level)
	{
		int pos = btree_find_pos( p_bnode, key );
		if( pos == -1 )
			return -1;
		return do_btree_find( inode, bt_root + p_bnode->kv[pos].value, level, key );
	}
	else if( p_bnode->level == level )
	{
		int pos = btree_find_pos( p_bnode, key );
		if( pos == -1 )
			return -1;
		if( p_bnode->kv[pos].key == key )
			return p_bnode->kv[pos].value ;
		else 
			return -1;
	}
	else
		panic("do_btree_find error!\n");
	return -1;
}

int btree_remove( struct m_inode *inode, char *name )
{
	int ret = -1;
	char tmp[20] = {0};
	memcpy( tmp, name, 19 );
	u32 key = hash_string( tmp, HASH_TYPE );
	struct buffer_head *bh = get_inode_data( inode->dev, inode->inode_num );
	ret = do_btree_remove( inode, NULL, 0, key );
	bwrite( bh );
	return ret;
}
static int do_btree_remove( struct m_inode *inode, struct d_btree_node *p_bnode, int level, int key )
{
	struct buffer_head *bh = get_inode_data( inode->dev, inode->inode_num );
	struct d_btree_node *bt_root = btree_init( bh );
	brelse( bh );
	if( p_bnode == NULL )
		p_bnode = bt_root;
	if( p_bnode->level > level )
	{
		int pos = btree_find_pos( p_bnode, key );
		if( pos == -1 )
			return -1;
		return  do_btree_remove( inode, bt_root + p_bnode->kv[pos].value, level, key );
	}
	else if( p_bnode->level == level )
	{
		int pos = btree_find_pos( p_bnode, key );
		if( pos == -1 )
			return -1;
		if( p_bnode->kv[pos].key == key )
		{
			p_bnode->kv[pos].key = 0;
			return p_bnode->kv[pos].value ;
		}
		else 
			return -1;
	}
	else
		panic("do_btree_find error!\n");
	return -1;
}


int btree_insert( struct m_inode *inode, const char *name, u32 inode_num )
{
	int ret = -1;
	char tmp[20] = {0};
	memcpy( tmp, name, 19 );
	u32 key = hash_string( tmp, HASH_TYPE );
	struct buffer_head *bh = get_inode_data( inode->dev, inode->inode_num );
	ret = do_btree_insert( inode, NULL, 0, key, inode_num );
	bwrite( bh );
	return ret;
}

static int do_btree_insert( struct m_inode *inode, struct d_btree_node *p_bnode, int level, u32 key, u32 inode_num )
{
	struct buffer_head *bh = get_inode_data( inode->dev, inode->inode_num );
	struct d_btree_node *bt_root = btree_init( bh );
	brelse( bh );
	if( p_bnode == NULL )
	{
		p_bnode = bt_root;
	}
	if( p_bnode->level > level )
	{
		int pos = btree_find_pos( p_bnode, key );
		if( pos == -1 )
		{
			p_bnode->kv[0].key = key;
			pos = 0;
		}
		if( p_bnode->kv[pos].value > (1 << inode->size) )
		{
			struct m_inode *n_inode = new_inode( inode->dev, inode->size + 1 );
			mov_inode_data( n_inode, inode );	
			struct m_inode *p_inode = get_inode( inode->dev, btree_find( inode, ".." ) );
			btree_remove( p_inode, inode->name );
			btree_insert( p_inode, n_inode->name, inode->inode_num );	
		}

		if( 64 == do_btree_insert( inode, bt_root + p_bnode->kv[pos].value, level, key, inode_num ) )
		{
			do_btree_split( inode, p_bnode );
			return do_btree_insert( inode, NULL, 0, key, inode_num );
		}
	}
	else if( p_bnode->level == level )
		return insert_pair( p_bnode, key, inode_num );
	else
		panic("do_btree_insert error!\n");
	return -1;
}

static int get_inode_empty_sector( struct m_inode *inode )
{
	int i;
	for( i=0; i<(1<<inode->size); i++)
		if( ~inode->bitmap & (1<<i) )
		{
			inode->bitmap = inode->bitmap | (1<<i);
			inode->dirt = 1;
			return i;
		}
	return -1 ;
}

static int do_btree_split_root( struct m_inode *inode, struct d_btree_node *p_bnode )
{
	int i,left,right;
	struct buffer_head *bh = get_inode_data( inode->dev, inode->inode_num );
	struct d_btree_node *bt_root = btree_init( bh );
	left = get_inode_empty_sector( inode );
	right = get_inode_empty_sector( inode );
	struct d_btree_node *p_left = bt_root + left;
	struct d_btree_node *p_right = bt_root + right;
	for( i=0; i<p_bnode->num/2; i++)
	{
		p_left->isroot = 0;
		p_left->level = p_bnode->level;
		p_left->kv[i] = p_bnode->kv[i];
		p_bnode->kv[i].key = 0; 
		p_left->num++;
		p_left->next_node_index = right;
	}
	for( ; i<p_bnode->num; i++ )
	{
		p_right->isroot = 0;
		p_right->level = p_bnode->level;
		p_right->kv[i] = p_bnode->kv[i];
		p_bnode->kv[i].key = 0; 
		p_right->num++;
		p_right->prev_node_index = left;
	}
	p_bnode->level++;
	p_bnode->num = 0;
	if( insert_pair( p_bnode, p_left->kv[0].key, left ) == -1)
		return -1;
	if( insert_pair( p_bnode, p_right->kv[0].key, right ) == -1 );
		return -1;
}
static int do_btree_split( struct m_inode *inode, struct d_btree_node *p_bnode)
{
	struct buffer_head *bh = get_inode_data( inode->dev, inode->inode_num );
	struct d_btree_node *bt_root = btree_init( bh );
	brelse( bh );
	int i;
	if( p_bnode->isroot == 1 )
	{
		do_btree_split_root( inode, p_bnode);
		return 0;
	}
	int right = get_inode_empty_sector( inode );
	struct d_btree_node *p_right = bt_root + right;
	for( i=p_bnode->num/2; i<p_bnode->num; i++ )
	{
		p_right->kv[i].key = p_bnode->kv[i].key;
		p_bnode->kv[i].key = 0;
		p_right->kv[i].value = p_bnode->kv[i].value;
		p_bnode->kv[i].value = 0;
		p_right->num++;
		p_bnode->num--;
		p_right->level = p_bnode->level;

		p_right->next_node_index = p_bnode->next_node_index;
		p_right->prev_node_index = p_bnode - bt_root;
		(bt_root + p_bnode->next_node_index)->prev_node_index = p_right - bt_root;
		p_bnode->next_node_index = p_right - bt_root;
	}
	return do_btree_insert( inode, NULL, p_bnode->level + 1, p_right->kv[0].key, p_right->kv[0].value );
}

static int btree_find_pos( struct d_btree_node *p_bnode, u32 key )
{
	int i;
	if( p_bnode->num == 0 )
		return -1;
	for( i=0; i<p_bnode->num; i++ )
		if( p_bnode->kv[i].key > key )
			return i-1;
	return p_bnode->num - 1;	
}
static int insert_pair( struct d_btree_node *p_bnode, u32 key, u32 value )
{
	if( p_bnode->num == 63 )
		return -1;
	int i,j;
	i = btree_find_pos( p_bnode, key );
	for( j=p_bnode->num; j>i+1; j-- )
	{
		p_bnode->kv[j].key = p_bnode->kv[j-1].key;
		p_bnode->kv[j].value = p_bnode->kv[j-1].value;
	}
	p_bnode->kv[i+1].key = key;
	p_bnode->kv[i+1].value = value;
	return p_bnode->num++;
}
