#include "lib/stdio.h"

int main( int argc, char *argv[] )
{
	int i;
	printf("%d\n", argc );
	
	printf("hello world\n");
	for( i=0; i<argc; i++ )
		printf("%s%s", " ", argv[i]);
	printf("\n");
	return 0;
}
