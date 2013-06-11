#include "stddef.h"
#include "proc.h"
#include "sched.h"

PROCESS* p_proc_ready;
void schedule()
{
	PROCESS* p;
	int greatest_ticks=0;

	while(!greatest_ticks)
	{
		for(p=proc_table;p<proc_table+NR_TASKS+NR_PROCS;p++)
		{
			if(p->flags==0)
			{
				if(p->ticks>greatest_ticks)
				{
					greatest_ticks=p->ticks;
					p_proc_ready=p;
				}
			}
		}
		if(!greatest_ticks)
		{
			for(p=proc_table;p<proc_table+NR_TASKS+NR_PROCS;p++)
				if(p->flags==0)
					p->ticks=p->priority;
		}
	}
}

void sleep_on( struct proc **p )
{
	struct proc *tmp;

	if( !p )
		return ;
	
	tmp = *p;
	*p = current_proc;
	current_proc->flags = SLEEP;
	schedule();
	if( tmp )
		tmp->flags = 0;
}

void wake_up( struct proc **p )
{
	if( p && *p )
	{
		(**p).flags = 0 ;
		p = NULL;
	}
}
