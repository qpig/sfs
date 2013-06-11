#include "fs.h"
#include "proc.h"

int alloc_mem( int pid, int memsize );
int free_mem( int pid );
static void cleanup( struct proc *proc );
int do_fork( MESSAGE *msg )
{
	struct proc *p = proc_table;
	int i;
	for( i=0; i<NR_TASKS+NR_PROCS; i++,p++ )
		if(p->flags == FREE_SLOT )
			break;
	int child_pid = i;
	assert(p == proc_table + child_pid );
	assert( child_pid >= NR_TASKS+NR_NATIVE_PROCS );
	if( i == NR_TASKS + NR_PROCS )
		return -1;
	assert( i < NR_TASKS + NR_PROCS );

	int pid = msg->source;
	u16 child_ldt_sel = p->ldt_sel;
	*p = proc_table[pid];
	p->ldt_sel = child_ldt_sel;
	p->parent = pid;
	sprintf( p->name, "%s_%d", proc_table[pid].name, child_pid );

	struct descriptor *ppd;
	ppd = &(proc_table[pid].s_ldts[INDEX_LDT_C]);
	int caller_T_base = (ppd->base_high << 24) + (ppd->base_mid << 16) + ppd->base_low ;
	int caller_T_limit = ((ppd->limit_high_attr2 & 0xF) << 16) + ppd->limit_low ;
	int caller_T_size = ( caller_T_limit + 1 ) * ((ppd->limit_high_attr2 & (DA_LIMIT_4K>>8)) ? 4096 : 1) ;

	ppd = &(proc_table[pid].s_ldts[INDEX_LDT_RW]);
	int caller_D_S_base = (ppd->base_high << 24) + (ppd->base_mid << 16) + ppd->base_low;
	int caller_D_S_limit = ((ppd->limit_high_attr2 & 0xF) << 16) + ppd->limit_low ;
	int caller_D_S_size = ( caller_D_S_limit + 1 ) * ((ppd->limit_high_attr2 & (DA_LIMIT_4K>>8)) ? 4096 : 1) ;

	assert( caller_T_base == caller_D_S_base && caller_T_size == caller_D_S_size);

	int child_base = alloc_mem( child_pid, caller_T_size );
	printk("MM 0x%x <-0x%x (0x%x bytes)\n", child_base, caller_T_base, caller_T_size );
	memcpy( (void *)child_base, (void *)caller_T_base, caller_T_size );

	init_descriptor( &p->s_ldts[INDEX_LDT_C], child_base, (PROC_IMAGE_SIZE_DEFAULT-1) >> 12, DA_LIMIT_4K | DA_32 | DA_C | PRIVILEGE_USER << 5 );
	init_descriptor( &p->s_ldts[INDEX_LDT_RW], child_base, (PROC_IMAGE_SIZE_DEFAULT-1) >> 12, DA_LIMIT_4K | DA_32 | DA_DRW | PRIVILEGE_USER << 5 );
	MESSAGE msg2fs;
	msg2fs.type = FORK;
	msg2fs.PID = child_pid;
	send_recv( BOTH, TASK_FS, &msg2fs );

	msg->PID = child_pid;

	MESSAGE child_msg;
	child_msg.type = SYSCALL_RET;
	child_msg.RETVAL = 0;
	child_msg.PID = 0;
	send_recv( SEND, child_pid, &child_msg );
	
	return 0;
}


void do_exit( MESSAGE *msg, int status )
{
	int i;
	int pid = msg->source;
	int parent_pid = proc_table[pid].parent;
	struct proc *p = proc_table + pid;

	MESSAGE msg2fs;
	msg2fs.type = EXIT;
	msg2fs.PID = pid;
	send_recv( BOTH, TASK_FS, &msg2fs );

	free_mem(pid);
	p->exit_status = status;
	
	if( proc_table[parent_pid].flags & WAITING )
	{
		proc_table[parent_pid].flags &= ~WAITING;
		cleanup( proc_table + pid );
	}
	else
	{
		proc_table[pid].flags |= HANGING ;
	}

	for( i=0; i<NR_TASKS + NR_PROCS; i++ )
	{
		if( proc_table[i].parent == pid )
		{
			proc_table[i].parent = INIT;
			if( (proc_table[INIT].flags & WAITING) && (proc_table[i].flags & HANGING) )
			{
				proc_table[INIT].flags &= ~WAITING;
				cleanup( proc_table + i );
			}
		}
	}
}

static void cleanup( struct proc *proc )
{
	MESSAGE msg2parent;
	msg2parent.type = SYSCALL_RET;
	msg2parent.PID = proc2pid(proc);
	msg2parent.STATUS = proc->exit_status;
	send_recv( SEND, proc->parent, &msg2parent );
	
	//memset( proc, 0, )
	//proc->flags = FREE_SLOT;
}

void do_wait( MESSAGE *msg)
{
	int pid = msg->source;
	int i;
	int children = 0;
	struct proc *p = proc_table;
	for( i=0; i<NR_TASKS + NR_PROCS; i++,p++ )
	{
		if( p->parent == pid )
		{
			children++;
			if( p->flags & HANGING )
			{
				cleanup( p );
				return ;
			}
		}
	}
	if(children)
	{
		proc_table[pid].flags |= WAITING;
	}
	else
	{
		MESSAGE m;
		m.type = SYSCALL_RET;
		m.PID = NO_TASK;
		send_recv( SEND, pid, &m );
	}
}
