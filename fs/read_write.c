#include "fs.h"

int rw_sector( int io_type, struct buffer_head *bh )
{
	if( bh->blocknr == 0 )
		return 0 ;
	MESSAGE driver_msg;
	
	assert( bh->dev == ROOT_DEV );
	driver_msg.source = TASK_FS;
	driver_msg.type = io_type;
	driver_msg.DEVICE = MINOR(bh->dev);
	driver_msg.POSITION = bh->blocknr;
	driver_msg.CNT =  1<<(bh->size);
	driver_msg.PROC_NR = TASK_FS;
	driver_msg.BUF = bh->pdata;

	send_recv( BOTH, dd_map[MAJOR(bh->dev)].driver_nr, &driver_msg );
	return 0;
}

int do_rdwr( MESSAGE *msg )
{
	int fd = msg->FD;
	int type = msg->type;
	void *buf = msg->BUF;
	int len = msg->CNT;
	int src = msg->source;
	struct proc *p = proc_table + src;
	assert( p->filp[fd] >= file_desc_table && p->filp[fd] < file_desc_table + NR_FILE_DESC );
	if( !(p->filp[fd]->fd_mode & O_RDWR) )
		return -1;
	int pos = p->filp[fd]->fd_pos;
	struct m_inode *inode = p->filp[fd]->fd_inode;
	assert( inode >= inode_table && inode < inode_table + NR_INODE );

	int imode = inode->mode &I_TYPE_MASK;
	if( imode == I_CHAR_SPECIAL )
	{
		msg->type = type == READ ? DEV_READ : DEV_WRITE;
		int dev = inode->start_data_sect;
		assert( MAJOR(dev) == 4 );
		msg->DEVICE = MINOR(dev);
		msg->BUF = buf;
		msg->CNT = len;
		msg->PROC_NR = src;
		send_recv( BOTH, dd_map[MAJOR(dev)].driver_nr, msg );
		return msg->CNT;	
	}
	/*
	else if( inode->mode == I_DIRECTORY )
	{
		len = MIN( len, SECTOR_SIZE *(1<<inode->size) );
		struct buffer_head *bh = get_inode_data( inode->dev, inode->inode_num );
		if( type == READ )
		{
			memcpy( (void *)va2la(src,buf), (void *)va2la(TASK_FS,bh->pdata+pos), len );
		}
		p->filp[fd]->fd_pos += len;
	}
	*/
	else
	{
	//	assert( inode->mode == I_REGULAR );
		assert( type == READ || type == WRITE );
		
		len = MIN( len, SECTOR_SIZE *(1<<inode->size) );
		struct buffer_head *bh = get_inode_data( inode->dev, inode->inode_num );
		if( type == READ )
		{
			memcpy( (void *)va2la(src,buf), (void *)va2la(TASK_FS,bh->pdata+pos), len );
		}
		else
		{
			memcpy( (void *)va2la(TASK_FS,bh->pdata+pos), (void *)va2la(src,buf), len );
		}
		p->filp[fd]->fd_pos += len;
	}
	return len;
}
