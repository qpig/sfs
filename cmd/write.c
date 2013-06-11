#include "lib/fcntl.h"
#include "lib/stdio.h"
#include "string.h"

int main( int argc, char *argv[] )
{
	int i;
	char buf[512] = {0};
	char file_name[128];
	int pos=0;
	printf("%d\n", argc );
	
	if( argc < 3 )
	{
		printf("argc must more than 3.\n");
		return -1;
	}
	for( i=0; i<argc; i++ )
		printf("%s%s", " ", argv[i]);
	printf("\n");
	for( i=2; i<argc; i++ )
	{
		sprintf( buf+pos, "%s%s", " ", argv[i]);
		pos+=strlen(argv[i]+1);
	}
	memcpy( file_name, argv[1], 128 );
	printf( "filename:%s\n",file_name);
	int fd = open( file_name, O_RDWR);
	write( fd, buf, 512 );
	close( fd );
	return 0;
}
