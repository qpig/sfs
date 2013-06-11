#include "type.h"
#include "fs.h"
#include "string.h"

int open( const char *pathname, int flags )
{
	MESSAGE msg;

	msg.type = OPEN;
	msg.PATHNAME = (void *)pathname;
	msg.FLAGS = flags;
	msg.NAME_LEN = strlen(pathname);

	send_recv( BOTH, TASK_FS, &msg );

	return msg.FD;
}
