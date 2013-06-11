#include "port.h"
#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "proto.h"
#include "lib/stdio.h"

void spin(char *func_name)
{
	printk("\nspinning in %s ...\n",func_name);
	while(1)
	{}
}

void  assertion_failure(char *exp,char *file,char *base_file,int line)
{
	printk("%c  assert(%s) failed: file: %s, base_file: %s, in %d",MAG_CH_ASSERT,exp,file,base_file,line);
	spin("assertion_failure()");
//	__asm__ __volatile__("ud2");
}

void panic(const char *fmt,...)
{
	int i;
	char buf[256];
	va_list arg=(va_list)((char *)&fmt+4);
	i = vsprintf(buf,fmt,arg);
	printk("%c panic %s:%d",MAG_CH_PANIC,buf,i);
}

int send_recv(int function,int src_dest,MESSAGE *msg)
{
	int ret=0;
	if(function==RECEIVE)
		memset(msg,0,sizeof(MESSAGE));
	switch(function)
	{
		case BOTH:
			ret=sendrec(SEND,src_dest,msg);
			if(ret==0)
				ret=sendrec(RECEIVE,src_dest,msg);
			break;
		case SEND:
		case RECEIVE:
			ret = sendrec(function,src_dest,msg);
			break;
		default:
			assert((function==BOTH) ||
				(function== SEND) ||
				(function == RECEIVE));
			break;
	}
	return ret;
}

