#include "proc.h"


int do_fork( MESSAGE *msg );
void do_exit( MESSAGE *msg, int status );
void do_wait( MESSAGE *msg);
void task_mm()
{
	MESSAGE msg;
	while(1)
	{
		send_recv( RECEIVE, ANY, &msg );
		int src = msg.source;
		int reply = 1;
		switch( msg.type )
		{
			case FORK:
				msg.RETVAL = do_fork( &msg );
				break;
			case EXIT:
				do_exit( &msg, msg.STATUS );
				reply = 0;
				break;
			case EXEC:
				do_exec( &msg );
				break;
			case WAIT:
				do_wait( &msg );
				reply = 0;
				break;
			default:
				dump_msg("MM:unknown msg", &msg );
				assert(0);
				break;
		}
		if( reply )
		{
			msg.type = SYSCALL_RET;
			send_recv( SEND, src, &msg );
		}
	}
}

int alloc_mem( int pid, int memsize )
{
	assert(pid >= (NR_TASKS + NR_NATIVE_PROCS));
	if (memsize > PROC_IMAGE_SIZE_DEFAULT) {
		panic("unsupported memory request: %d. "
			"(should be less than %d)",
			memsize,
			PROC_IMAGE_SIZE_DEFAULT);
	}

	int base = PROCS_BASE +	(pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_IMAGE_SIZE_DEFAULT;

	if (base + memsize >= 0x2000000)
		panic("memory allocation failed. pid:%d", pid);

	return base;
}

int free_mem( int pid )
{
	return 0;
}
