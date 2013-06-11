#ifndef _TTY_H_
#define _TTY_H_

#include "proc.h"
#define TTY_IN_BYTES 256
#define TTY_OUT_BUF_LEN     2 
extern int key_pressed ;

struct console;

typedef struct tty
{
	u32 in_buf[TTY_IN_BYTES];
	u32* p_inbuf_head;
	u32* p_inbuf_tail;
	int inbuf_count;

	int tty_caller;
	int tty_procnr;
	void *tty_req_buf;
	int tty_left_cnt;
	int tty_trans_cnt;

	struct console* p_console;
}TTY;

extern TTY tty_table[];
int sys_printx(int _unsued1,int _unused2,char *s,struct proc *p_proc);
void in_process(TTY* p_tty,u32 key);
void task_tty();

#endif
