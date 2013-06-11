#include "fs.h"

int do_unlink( int src, int name, int name_len)
{
	char path_name[MAX_PATH];
	assert( name_len < MAX_PATH );
	strncpy( (void *)va2la(TASK_FS,path_name), (void *)va2la(src,name), name_len );
	pathname[name_len] = 0;

	if( strcmp(pathname, "/") == 0 )
		printf("FS:do_unlink can't unlink root!\n");
	char *file_name = NULL;
	int dir_inode_nr = 0;
	int file_inode_nr = find_entry( path_name,  &file_name, &dir_inode_nr );
	struct m_inode *file_inode = get_inode( ROOT_DEV, file_inode_nr );

	if( file_inode->mode != I_REGULAR )
	{
		printf("can't remove file %s because it is not a regular file.\n", file_name );
		return -1;
	}
	if( file_inode->count > 1 )
	{
		printf("can't remove file %s because inode count is %s.\n", file_name, file_inode->count );
		retunr -1;
	}
	
}
