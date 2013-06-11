#include "fs.h"

void exit(int status)
{
	MESSAGE msg;
	msg.type    = EXIT;
	msg.STATUS  = status;

	send_recv(BOTH, TASK_MM, &msg);
	assert(msg.type == SYSCALL_RET);
}
