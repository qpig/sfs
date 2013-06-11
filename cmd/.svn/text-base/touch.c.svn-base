#include "lib/fcntl.h"
#include "lib/stdio.h"

int main( int argc, char *argv[] )
{
	int i;
	char buf[1024] = {0};
	char file_name[128];
	printf("%d\n", argc );
	
	for( i=0; i<argc; i++ )
		printf("%s%s", " ", argv[i]);
	printf("\n");
	if( argc < 2 )
	{
		printf("argc must more then 2.\n");
		return -1;
	}
	memcpy( file_name, argv[1], 128 );
	printf( "name:%s\n",file_name);
	int fd_dir = open( file_name, O_CREAT);
	close( fd_dir );
	return 0;
}
