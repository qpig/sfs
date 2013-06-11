#include "lib/fcntl.h"
#include "lib/stdio.h"
#include "fs.h"

int main( int argc, char *argv[] )
{
	int i;
	char buf[SECTOR_SIZE*2] = {0};
	char file_name[128]="/.";
	printf("%d\n", argc );
	
	for( i=0; i<argc; i++ )
		printf("%s%s", " ", argv[i]);
	printf("\n");


	if( argc >1 )
		memcpy( file_name, argv[1], 125 );
	int fd_dir = open( file_name, O_RDWR );
	i = read( fd_dir, buf, SECTOR_SIZE*2 );
	struct d_btree_node *btnode = buf;
	printf( "file number: %d\n", btnode->num );
	for( i=0; i<btnode->num; i++ )
	{
		int inode_nr=btnode->kv[i].value;
		printf( "key:%d,inode_nr:%d\n", btnode->kv[i].key, inode_nr);
	}
	close( fd_dir );
	return 0;
}
