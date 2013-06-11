#include "fs.h"


#define MAY_EXEC 1
#define MAY_WRITE 2
#define MAY_READ  4


int find_file_in_dir( char *name, int dev, int dir_inode_nr )
{
	struct m_inode *inode = get_inode( dev, dir_inode_nr );
	return btree_find( inode, name );
}
int find_entry( char *path_name, char **file_name, int *dir_inode_nr )
{
	char *name_end = NULL;
	if( *path_name!= '/' )
		return -1;
		//panic("file name must begin / ");
	*file_name = path_name+1;
	*dir_inode_nr = 1;
	while( (name_end=strchr(*file_name, '/')) != NULL )
	{
		*name_end = 0;
		printk( "dir is %s\n", *file_name );
		*dir_inode_nr=find_file_in_dir(*file_name, ROOT_DEV, *dir_inode_nr);
		if( *dir_inode_nr < 0 )
			return -1;
		*file_name = name_end+1 ;
	}
	int file_inode_nr = find_file_in_dir( *file_name , ROOT_DEV, *dir_inode_nr );
	printk( "name are %s\n", *file_name );
	printk( "file inode %d\n", file_inode_nr );
	return file_inode_nr;
}
