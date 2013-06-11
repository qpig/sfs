#include "fs.h"

int fork()
{
	MESSAGE msg;
	msg.type = FORK;

	send_recv( BOTH, TASK_MM, &msg );
	assert(msg.type == SYSCALL_RET);
	assert(msg.RETVAL == 0);

	return msg.PID;
}
