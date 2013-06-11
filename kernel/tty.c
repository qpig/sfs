#include "port.h"
#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "proto.h"
#include "keyboard.h"
#include "sched.h"

static void init_tty(TTY* p_tty);
static void put_key(TTY* tty,u32 key);
static void tty_dev_read(TTY* p_tty);
static void tty_dev_write(TTY* p_tty);
static void tty_do_read( TTY *p_tty, MESSAGE *msg );
static void tty_do_write( TTY *p_tty, MESSAGE *msg );
int key_pressed = 0;

TTY tty_table[NR_CONSOLES];

void task_tty()
{
	TTY* p_tty;
	MESSAGE msg;
	init_keyboard();

	for(p_tty=tty_table;p_tty<tty_table+NR_CONSOLES;p_tty++)
	{
		init_tty(p_tty);
	}
	select_console(0);
	while(1)
	{
		for(p_tty=tty_table;p_tty<tty_table+NR_CONSOLES;p_tty++)
		{
			tty_dev_read(p_tty);
			tty_dev_write(p_tty);
		}

		send_recv( RECEIVE, ANY, &msg );
		int src = msg.source;
		assert( src != TASK_TTY );
		TTY *ptty = tty_table + msg.DEVICE ;
		switch(msg.type)
		{
			case DEV_OPEN:
				reset_msg(&msg);
				msg.type = SYSCALL_RET;
				send_recv( SEND,src, &msg );
				break;
			case DEV_READ:
				tty_do_read( ptty, &msg );
				break;
			case DEV_WRITE:
				tty_do_write( ptty, &msg );
				break;
			case HARD_INT:
				key_pressed = 0;
				break;
			default:
				dump_msg("TTY:unknown msg", &msg );
				break;
		}
	}
}

static void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count=0;
	p_tty->p_inbuf_head=p_tty->p_inbuf_tail=p_tty->in_buf;
	init_screen(p_tty);
}

static void tty_dev_read(TTY* p_tty)
{
	if(is_current_console(p_tty->p_console))
	{
		keyboard_read(p_tty);
	}
}

static void tty_dev_write(TTY* p_tty)
{
	while(p_tty->inbuf_count>0)
	{
		char ch=*(p_tty->p_inbuf_tail++);
		if(p_tty->p_inbuf_tail == p_tty->in_buf+TTY_IN_BYTES)
		{
			p_tty->p_inbuf_tail=p_tty->in_buf;
		}
		p_tty->inbuf_count--;
		if( p_tty->tty_left_cnt)
		{
			if( ch >= ' ' && ch <= '~' )
			{
				out_char(p_tty->p_console,ch);
				void *p = p_tty->tty_req_buf + p_tty->tty_trans_cnt;
				memcpy( p, (void *)va2la(TASK_TTY,&ch), 1 );
				p_tty->tty_trans_cnt ++;
				p_tty->tty_left_cnt --;
			}
			else if( ch == '\b' && p_tty->tty_trans_cnt )
			{
				out_char( p_tty->p_console, ch );
				p_tty->tty_trans_cnt --;
				p_tty->tty_left_cnt ++;
			}
			if( ch == '\n' || p_tty->tty_left_cnt == 0 )
			{
				out_char( p_tty->p_console, '\n' );
				MESSAGE msg;
				msg.type = RESUME_PROC;
				msg.PROC_NR = p_tty->tty_procnr;
				msg.CNT = p_tty->tty_trans_cnt;
				send_recv( SEND, p_tty->tty_caller, &msg );
				p_tty->tty_left_cnt = 0;
			}
		}
	}
}

static void tty_do_read( TTY *p_tty, MESSAGE *msg )
{
	p_tty->tty_caller = msg->source;
	p_tty->tty_procnr = msg->PROC_NR;
	p_tty->tty_req_buf = va2la( p_tty->tty_procnr, msg->BUF );
	p_tty->tty_left_cnt = msg->CNT;
	p_tty->tty_trans_cnt = 0;
	msg->type = SUSPEND_PROC;
	msg->CNT = p_tty->tty_left_cnt;
	send_recv( SEND, p_tty->tty_caller, msg );
}

static void tty_do_write( TTY *p_tty, MESSAGE *msg )
{
	char buf[TTY_OUT_BUF_LEN];
	char *p = (char *)va2la( msg->PROC_NR, msg->BUF );
	int i = msg->CNT;
	int j;
	while(i)
	{
		int bytes = MIN( TTY_OUT_BUF_LEN, i );
		memcpy( va2la( TASK_TTY, buf), (void *)p, bytes );
		for( j=0; j<bytes; j++ )
			out_char( p_tty->p_console, buf[j] );
		i -= bytes;
		p += bytes;
	}
	msg->type = SYSCALL_RET;
	send_recv( SEND, msg->source, msg );
}

/*
void tty_write(TTY* p_tty,char* buf,int len)
{
	char* p=buf;
	int i=len;
	while(i--)
	{
		out_char(p_tty->p_console,*p++);
	}
}
*/

static void put_key(TTY* p_tty,u32 key)
{
		if(p_tty->inbuf_count<TTY_IN_BYTES)
		{
			*(p_tty->p_inbuf_head++)=key;
			if(p_tty->p_inbuf_head==p_tty->in_buf+TTY_IN_BYTES)
			{
				p_tty->p_inbuf_head=p_tty->in_buf;
			}
			p_tty->inbuf_count++;
		}
}

void in_process(TTY* p_tty,u32 key)
{
	if(!(key & FLAG_EXT))
	{
		put_key(p_tty,key);
	}
	else
	{
		int raw_code=key& MASK_RAW;
		switch(raw_code)
		{
			case ENTER:
				put_key(p_tty,'\n');
				break;
			case BACKSPACE:
				put_key(p_tty,'\b');
				break;
			case UP: 
				if((key & FLAG_SHIFT_L) || (key& FLAG_SHIFT_R))
				{
					scroll_screen(p_tty->p_console,SCR_UP);
				}
				break;
			case DOWN:
				if((key & FLAG_SHIFT_L) || (key& FLAG_SHIFT_R))
				{
					scroll_screen(p_tty->p_console,SCR_DN);
				}
				break;
			case F1:
			case F2:
			case F3:
			case F4:
			case F5:
			case F6:
			case F7:
				if((key & FLAG_ALT_L) || (key & FLAG_ALT_R))
				{
					select_console(raw_code-F1);
				}
				break;
		}
	}
}

int sys_printx(int _unsued1,int _unused2,char *s,struct proc *p_proc)
{
	const char *p;
	char ch;
	char reenter_err[]=" k_reenter is incorrrect for unknown reason";
	reenter_err[0]=MAG_CH_PANIC;
	
	if(k_reenter==0)
		p=va2la(proc2pid(p_proc),s);
	else if(k_reenter>0)
		p=s;
	else
		p=reenter_err;

	if((*p==MAG_CH_PANIC) || 
			(*p==MAG_CH_ASSERT && p_proc_ready<&proc_table[NR_TASKS]))
	{
		disable_int();
		char *v=(char*)V_MEM_BASE;
		const char* q=p+1;
		while(((int)v-V_MEM_BASE) <(SCREEN_WIDTH*26))
		{
			v++;
			*v++=GRAY_CHAR;
		}
		while(v<(char*)(V_MEM_BASE+V_MEM_SIZE) && (*q != '\0') )
		{
			*v++=*q++;
			*v++=RED_CHAR;
		}
		__asm__ __volatile__("hlt");
	}
	while((ch=*p++)!=0)
	{
		if(ch== MAG_CH_PANIC || ch== MAG_CH_ASSERT)
		{
			continue;
		}
		out_char(tty_table[p_proc->nr_tty].p_console,ch);
	}
	return 0;
}
