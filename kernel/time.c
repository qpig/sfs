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
#include "sched.h"
#include "i8259.h"
#include "tty.h"

int ticks = 0;
void clock_handler(int irq)
{
	ticks++;
	if( ticks >= MAX_TICKS )
		ticks = 0;
	if( p_proc_ready->ticks)
		p_proc_ready->ticks--;
	if( key_pressed )
		inform_int( TASK_TTY );
	if(k_reenter!=0)
	{
		return;
	}
	schedule();
}

void milli_delay(int milli_sec)
{
	int t=get_ticks();
	while(((get_ticks()-t) *1000 /HZ) < milli_sec){}
}

void init_clock()
{
	/*init 8253*/
	out_byte(TIMER_MODE_REG,RATE_GENERATOR);
	out_byte(TIMER0_REG,(u8) (TIMER_FREG/HZ));
	out_byte(TIMER0_REG,(u8)((TIMER_FREG/HZ)>>8));

	put_irq_handler(CLOCK_IRQ,clock_handler);
	enable_irq(CLOCK_IRQ);
}

int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH,TASK_SYS,&msg);
	return msg.RETVAL;
}
