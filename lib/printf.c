#include "fs.h"
#include "proto.h"


int printf( const char *fmt, ... )
{
	int i;
	char buf[256];
	
	va_list arg = (va_list)((char *)(&fmt) + 4);
	i = vsprintf( buf, fmt, arg );
	buf[i] = 0;

	int c = write( 1, buf, i );
	assert( c == i );
	
	return i;
}

int printk(const char *fmt, ...)
{
	int i;
	char buf[256];
	va_list arg=(va_list)((char* )(&fmt)+4);
	i=vsprintf(buf,fmt,arg);
	buf[i]=0;
	printx(buf);
	return i;
}
