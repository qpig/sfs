#include "port.h"
#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "proto.h"
#include "time.h"
#include "errno.h"
#include "lib/stdio.h"

void task_sys()
{
	printk("task_sys begin\n");
	MESSAGE msg;
	while(1)
	{
		send_recv(RECEIVE,ANY,&msg);
		int src=msg.source;
		switch(msg.type)
		{
			case GET_TICKS:
				msg.RETVAL=ticks;
				send_recv(SEND,src,&msg);
				break;
			case GET_PID:
				msg.type = SYSCALL_RET;
				msg.PID = src;
				send_recv( SEND, src, &msg );
				break;
			default:
				panic("unknown msg type");
				break;
		}
	}
}
