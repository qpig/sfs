#include "port.h"
#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "proto.h"
#include "sched.h"
#include "errno.h"
#include "lib/stdio.h"

PROCESS proc_table[NR_TASKS+ NR_PROCS];
int k_reenter = 0;

TASK task_table[NR_TASKS]=
{
	{task_tty,STACK_SIZE_TTY,"TTY"},
	{task_sys,STACK_SIZE_SYS,"SYS"},
	{task_hd, STACK_SIZE_HD, "HD"},
	{task_fs, STACK_SIZE_FS, "FS"},
	{task_mm, STACK_SIZE_MM, "MM"}
};
TASK user_proc_table[NR_NATIVE_PROCS]=
{
	{Init, STACK_SIZE_INIT, "INIT"},
	{TestA,STACK_SIZE_TESTA,"TestA"},
	{TestB,STACK_SIZE_TESTB,"TestB"},
	{TestC,STACK_SIZE_TESTC,"TestC"}
};
char task_stack[STACK_SIZE_TOTAL];
system_call sys_call_table[NR_SYS_CALL]={sys_printx,
										sys_sendrec};

static void block(struct proc *p);
static void unblock(struct proc *p);
static int deadlock(int src,int dest);
static int msg_send(struct proc *current,int dest,MESSAGE *m);
static int msg_receive(struct proc *current,int src,MESSAGE *m);
int sendrecv(int function,int src_dest,MESSAGE *msg);

void init_proc()
{
	TASK* p_task = task_table;
	PROCESS* p_proc = proc_table;
	u8 privilege;
	u8 rpl;
	int i,j,eflags,prio;
	char* p_task_stack=task_stack+STACK_SIZE_TOTAL;
	for(i=0;i<NR_TASKS+NR_PROCS;i++)
	{
		if( i>= NR_TASKS + NR_NATIVE_PROCS )
		{
			p_proc->flags = FREE_SLOT;
			p_proc++;
			continue;
		}
		if(i<NR_TASKS)
		{
			p_task=task_table+i;
			privilege=PRIVILEGE_TASK;
			rpl=RPL_TASK;
			eflags=0x1202;
			prio=15;
		}
		else
		{
			p_task=user_proc_table+(i-NR_TASKS);
			privilege=PRIVILEGE_USER;
			rpl=RPL_USER;
			eflags=0x202;
			prio=5;
		}
		strcpy(p_proc->name,p_task->name);
		p_proc->pid=i;
		p_proc->parent = NO_TASK;
		if( strcmp( p_task->name, "INIT") != 0 )
		{
			p_proc->s_ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS>>3];
			p_proc->s_ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS>>3];

			p_proc->s_ldts[INDEX_LDT_C].attr1 = DA_C | privilege << 5;
			p_proc->s_ldts[INDEX_LDT_RW].attr1 = DA_DRW | privilege << 5;
		}
		else
		{
			int k_base = 0;
			int k_limit = 0xD0000;
			init_descriptor( &p_proc->s_ldts[INDEX_LDT_C], 0, (k_base+k_limit)>>12, DA_32 | DA_LIMIT_4K | DA_C | privilege <<5);
			init_descriptor( &p_proc->s_ldts[INDEX_LDT_RW], 0, (k_base+k_limit)>>12, DA_32 | DA_LIMIT_4K | DA_DRW | privilege <<5);
		}
		p_proc->s_regs.cs=(0 & SA_RPL_MASK & SA_TI_MASK )| SA_TIL | rpl;
		p_proc->s_regs.ds=(8 & SA_RPL_MASK & SA_TI_MASK ) | SA_TIL| rpl;
		p_proc->s_regs.es=(8 & SA_RPL_MASK & SA_TI_MASK ) | SA_TIL |rpl;
		p_proc->s_regs.fs=(8 & SA_RPL_MASK & SA_TI_MASK ) | SA_TIL | rpl;
		p_proc->s_regs.ss=(8 & SA_RPL_MASK & SA_TI_MASK ) | SA_TIL | rpl;
		p_proc->s_regs.gs=(SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p_proc->s_regs.eip=(u32) p_task->initial_eip;
		p_proc->s_regs.esp=(u32) p_task_stack;
		p_proc->s_regs.eflags=eflags;

		p_proc->nr_tty=0;
		p_proc->flags=0;
		p_proc->p_msg=0;
		p_proc->recvfrom=NO_TASK;
		p_proc->sendto=NO_TASK;
		p_proc->has_int_msg=0;
		p_proc->q_sending=0;
		p_proc->next_sending=0;
		p_proc->ticks = p_proc->priority=prio;
		p_task_stack-=p_task->stacksize;
		
		p_proc->nr_tty = 0;

		for( j=0; j<NR_FILES; j++ )
			p_proc->filp[j] = 0;
		p_proc++;
	}
	p_proc_ready = proc_table;
}

int sys_sendrec(int function,int src_dest,MESSAGE* m,struct proc*p)
{
	assert(k_reenter==0);
//	printk("%d,",src_dest );
	assert((src_dest >= 0 && src_dest < NR_TASKS + NR_PROCS)
			|| src_dest == ANY || src_dest == INTERRUPT);
	int ret=0;
	int caller=proc2pid(p);
	MESSAGE* mla=(MESSAGE*)va2la(caller,m);
	mla->source=caller;
	assert(mla->source != src_dest);
	if(function == SEND)
	{
		ret=msg_send(p,src_dest,m);
		if(ret !=0)
			return ret;
	}
	else if(function == RECEIVE)
	{
		ret=msg_receive(p,src_dest,m);
		if(ret !=0)
			return ret;
	}
	else
	{
		panic("sys_sendrec invalid function: %d (SEND: %d, RECEIVE: %d).",
				function,SEND,RECEIVE);
	}
	return 0;
}

void *va2la(int pid,void *va)
{
	struct proc *p=&proc_table[pid];
	u32 seg_base=ldt_seg_linear(p,INDEX_LDT_RW);
	u32 la=seg_base +(u32)va;
	if(pid<NR_TASKS+NR_NATIVE_PROCS)
	{
		assert(la==(u32)va);
	}
	return (void*)la;
}

int ldt_seg_linear(struct proc *p,int idx)
{
	struct descriptor *d=&(p->s_ldts[idx]);
	return d->base_high<<24 | d->base_mid<<16 | d->base_low;
}

void reset_msg(MESSAGE *p)
{
	memset( p, 0, sizeof(MESSAGE) );
}

static void block(struct proc *p)
{
	assert(p->flags);
	schedule();
}

static void unblock(struct proc *p)
{
	assert(p->flags==0);
}

static int deadlock(int src,int dest)
{
	struct proc *p=proc_table+dest;
	while(1)
	{
		if(p->flags & SENDING)
		{
			if(p->sendto ==src)
			{
				p=proc_table+dest;
				printk("=_=%s",p->name);
				do
				{
					assert(p->p_msg);
					p=proc_table+p->sendto;
					printk("->%s",p->name);
				}while(p!=proc_table+src);
				printk("=_=");
				return 1;
			}
			p=proc_table+p->sendto;
		}
		else
		{
			break;
		}
	}
	return 0;
}


static int msg_send(struct proc *current,int dest,MESSAGE *m)
{
	struct proc *sender=current;
	struct proc *p_dest=proc_table+dest;
	assert(proc2pid(sender)!=dest);
	if(deadlock(proc2pid(sender),dest))
	{
		panic(">>DEADLOCK<< %s->%s",sender->name,p_dest->name);
	}
	if((p_dest->flags & RECEIVING) && 
			(p_dest->recvfrom == proc2pid(sender) || p_dest->recvfrom== ANY))
	{
		assert(p_dest->p_msg);
		assert(m);

		memcpy(va2la(dest,p_dest->p_msg),
				va2la(proc2pid(sender),m),
				sizeof(MESSAGE));
		p_dest->p_msg=0;
		p_dest->flags&=~RECEIVING;
		p_dest->recvfrom=NO_TASK;
		unblock(p_dest);
		assert(p_dest->flags ==0);
		assert(p_dest->p_msg ==0);
		assert(p_dest->recvfrom==NO_TASK);
		assert(p_dest->sendto ==NO_TASK);
		assert(sender->flags==0);
		assert(sender->p_msg==0);
		assert(sender->recvfrom==NO_TASK);
		assert(sender->sendto==NO_TASK);
	}
	else
	{
		sender->flags |= SENDING;
		
		sender->sendto=dest;
		sender->p_msg=m;
		struct proc *p;
		if(p_dest->q_sending)
		{
			p=p_dest->q_sending;
			while(p->next_sending)
			{
				p=p->next_sending;
			}
			p->next_sending=sender;
		}
		else
		{
			p_dest->q_sending=sender;
		}
		sender->next_sending=0;
		block(sender);

		assert(sender->flags==SENDING);
		assert(sender->p_msg!=0);
		assert(sender->recvfrom==NO_TASK);
		assert(sender->sendto==dest);
	}
	return 0;
}

static int msg_receive(struct proc *current,int src,MESSAGE *m)
{
	struct proc *p_who_recv=current;
	struct proc *p_from=0;
	struct proc *prev=0;
	int copyok=0;

	if((p_who_recv->has_int_msg) &&
			((src==ANY) || (src ==INTERRUPT)))
	{
		MESSAGE msg;
		reset_msg(&msg);
		msg.source=INTERRUPT;
		msg.type=HARD_INT;
		memcpy(va2la(proc2pid(p_who_recv),m),&msg,sizeof(MESSAGE));
		p_who_recv->has_int_msg=0;
		assert(m);
		assert(p_who_recv->flags==0);
		assert(p_who_recv->p_msg==0);
		assert(p_who_recv->sendto==NO_TASK);
		assert(p_who_recv->has_int_msg==0);
		return 0;
	}
	if(src==ANY)
	{
		if(p_who_recv->q_sending)
		{
			p_from=p_who_recv->q_sending;
			copyok=1;
		}
	}
	else if(src>=0 &&
			src< NR_TASKS+NR_PROCS)
	{
		p_from=&proc_table[src];
		if((p_from->flags & SENDING) &&
				(p_from->sendto == proc2pid(p_who_recv)))
		{
			copyok=1;
			struct proc *p=p_who_recv->q_sending;
			assert(p);
			while(p)
			{
				if(proc2pid(p) ==src)
					break;
				prev=p;
				p=p->next_sending;
			}

		}
	}
	if(copyok)
	{
		if(p_from ==p_who_recv->q_sending)
		{
			p_who_recv->q_sending=p_from->next_sending;
			p_from->next_sending=0;
		}
		else
		{
			prev->next_sending=p_from->next_sending;
			p_from->next_sending=0;
		}

		memcpy(va2la(proc2pid(p_who_recv),m),
				va2la(proc2pid(p_from),p_from->p_msg),
				sizeof(MESSAGE));
		p_from->p_msg=0;
		p_from->sendto=NO_TASK;
		p_from->flags&=~SENDING;
		unblock(p_from);

	}
	else
	{
		p_who_recv->flags|=RECEIVING;
		p_who_recv->p_msg=m;
		p_who_recv->recvfrom=src;
		block(p_who_recv);
	}

	return 0;
}


void inform_int(int task_nr)
{
	struct proc* p = proc_table + task_nr;

	if ((p->flags & RECEIVING) && /* dest is waiting for the msg */
	    ((p->recvfrom == INTERRUPT) || (p->recvfrom == ANY))) {
		p->p_msg->source = INTERRUPT;
		p->p_msg->type = HARD_INT;
		p->p_msg = 0;
		p->has_int_msg = 0;
		p->flags &= ~RECEIVING; /* dest has received the msg */
		p->recvfrom = NO_TASK;
		assert(p->flags == 0);
		unblock(p);

		assert(p->flags == 0);
		assert(p->p_msg == 0);
		assert(p->recvfrom == NO_TASK);
		assert(p->sendto == NO_TASK);
	}
	else {
		p->has_int_msg = 1;
	}
}

/*****************************************************************************
 *                                dump_proc
 *****************************************************************************/
void dump_proc(struct proc* p)
{
	char info[STR_DEFAULT_LEN];
	int i;
	int text_color = MAKE_COLOR(GREEN, RED);

	int dump_len = sizeof(struct proc);

	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, 0);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, 0);

	sprintf(info, "byte dump of proc_table[%d]:\n", p - proc_table); disp_color_str(info, text_color);
	for (i = 0; i < dump_len; i++) {
		sprintf(info, "%x.", ((unsigned char *)p)[i]);
		disp_color_str(info, text_color);
	}

	/* printl("^^"); */

	disp_color_str("\n\n", text_color);
	sprintf(info, "ANY: 0x%x.\n", ANY); disp_color_str(info, text_color);
	sprintf(info, "NO_TASK: 0x%x.\n", NO_TASK); disp_color_str(info, text_color);
	disp_color_str("\n", text_color);

	sprintf(info, "ldt_sel: 0x%x.  ", p->ldt_sel); disp_color_str(info, text_color);
	sprintf(info, "ticks: 0x%x.  ", p->ticks); disp_color_str(info, text_color);
	sprintf(info, "priority: 0x%x.  ", p->priority); disp_color_str(info, text_color);
	sprintf(info, "pid: 0x%x.  ", p->pid); disp_color_str(info, text_color);
	sprintf(info, "name: %s.  ", p->name); disp_color_str(info, text_color);
	disp_color_str("\n", text_color);
	sprintf(info, "flags: 0x%x.  ", p->flags); disp_color_str(info, text_color);
	sprintf(info, "recvfrom: 0x%x.  ", p->recvfrom); disp_color_str(info, text_color);
	sprintf(info, "sendto: 0x%x.  ", p->sendto); disp_color_str(info, text_color);
	sprintf(info, "nr_tty: 0x%x.  ", p->nr_tty); disp_color_str(info, text_color);
	disp_color_str("\n", text_color);
	sprintf(info, "has_int_msg: 0x%x.  ", p->has_int_msg); disp_color_str(info, text_color);
}


/*****************************************************************************
 *                                dump_msg
 *****************************************************************************/
void dump_msg(const char * title, MESSAGE* m)
{
	int packed = 0;
	printk("{%s}<0x%x>{%ssrc:%s(%d),%stype:%d,%s(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)%s}%s",  //, (0x%x, 0x%x, 0x%x)}",
	       title,
	       (int)m,
	       packed ? "" : "\n        ",
	       proc_table[m->source].name,
	       m->source,
	       packed ? " " : "\n        ",
	       m->type,
	       packed ? " " : "\n        ",
	       m->u.m3.m3i1,
	       m->u.m3.m3i2,
	       m->u.m3.m3i3,
	       m->u.m3.m3i4,
	       (int)m->u.m3.m3p1,
	       (int)m->u.m3.m3p2,
	       packed ? "" : "\n",
	       packed ? "" : "\n"/* , */
		);
}

