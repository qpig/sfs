#include "fs.h"
#include "string.h"
#include "lib/fcntl.h"

static struct m_inode *create_file( char *name, int dev, int dir_inode_nr, int size );

int do_open( MESSAGE *msg )
{
	int flags = msg->FLAGS;
	int name_len = msg->NAME_LEN ;
	int src = msg->source;
	char path_name[MAX_PATH];
	assert( name_len < MAX_PATH );
	memcpy( (void *)va2la( TASK_FS, path_name ), (void *)va2la( src, msg->PATHNAME ), name_len );
	path_name[name_len] = 0;
	int fd = -1;
	int i;
	char *file_name=NULL;
	struct proc *pcaller = proc_table + src ;
	for( i=0; i<NR_FILES; i++ )
		if( pcaller->filp[i] == 0 )
		{
			fd = i;
			break;
		}
	if( fd<0 || fd>= NR_FILES )
		panic(" filp[] is full (PID:%d)\n", proc2pid(pcaller) );
	for( i=0; i<NR_FILE_DESC; i++ )
		if( file_desc_table[i].fd_inode == 0 )
			break;
	if( i>NR_FILE_DESC )
		panic("file_desc_table[] is full (PID:%d)\n", proc2pid(pcaller) );
	int dir_inode_nr = 0;
	int file_inode_nr = find_entry( path_name, &file_name, &dir_inode_nr );
	struct m_inode *file_inode = NULL;
	if( flags & O_CREAT )
	{
		if( file_inode_nr > 0 )
		{
			printk(" file exists.\n");
			return -1;
		}
		file_inode = create_file( file_name, ROOT_DEV, dir_inode_nr, 0 );
	}
	else
	{
		assert( flags & O_RDWR );
		if( file_inode_nr <= 0 )
			return -1;
		file_inode = get_inode( ROOT_DEV, file_inode_nr );
	}
	if( file_inode != NULL)
	{
		pcaller->filp[fd] = file_desc_table+i;
		file_desc_table[i].fd_inode = file_inode;
		file_desc_table[i].fd_mode = flags;
		file_desc_table[i].fd_pos = 0;

		int imode = file_inode->mode & I_TYPE_MASK;
		if( imode == I_CHAR_SPECIAL )
		{
			MESSAGE msg;
			msg.type = DEV_OPEN;
			int dev = file_inode->start_data_sect;
			msg.DEVICE = MINOR(dev);
			assert( MAJOR(dev) == 4 );
			send_recv( BOTH, dd_map[MAJOR(dev)].driver_nr, &msg );
		}
		else if( imode == I_DIRECTORY )
		{
			;
		}
		else
		{
			assert( file_inode->mode == I_REGULAR );
		}
	}
	else
	{
		return -1;
	}
	return fd;
}

static struct m_inode *create_file( char *name, int dev, int dir_inode_nr, int size )
{
	struct m_inode *inode = new_inode( dev, size ); 
	strncpy( inode->name, name, 19 ); 
	inode->name[19] = 0;
	inode->mode = I_REGULAR ;
	struct m_inode *dir_inode = get_inode( dev, dir_inode_nr );
	btree_insert( dir_inode, name, inode->inode_num );
	put_inode( dir_inode );
	return inode;
}


int do_close( MESSAGE *msg )
{
	int fd = msg->FD;
	int src = msg->source;
	struct proc *p = proc_table + src;
	put_inode( p->filp[fd]->fd_inode );
	p->filp[fd]->fd_inode = 0;
	p->filp[fd] = 0 ;
	return 0;
}
