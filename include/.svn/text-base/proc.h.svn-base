#ifndef _PROC_H_
#define _PROC_H_
#include "type.h"
#include "protect.h"
#include "proto.h"
#include "fs.h"

#define PROCS_BASE 0xA00000
#define PROC_IMAGE_SIZE_DEFAULT 0x100000
#define PROC_STACK 0x400

#define SEND 1
#define RECEIVE 2
#define BOTH 3

#define INVALID_DRIVER -20
#define INTERRUPT -10
#define TASK_TTY 0
#define TASK_SYS 1
#define TASK_HD  2
#define TASK_FS  3
#define TASK_MM  4
#define INIT     5
#define ANY (NR_TASKS+NR_PROCS+10)
#define NO_TASK (NR_TASKS+NR_PROCS+20)

#define NR_SYS_CALL 2

//state
#define SENDING 0X02
#define RECEIVING 0x04
#define SLEEP	0x08
#define WAITING 0x08
#define HANGING 0x10
#define FREE_SLOT 0x20

#define	NR_FILES	64
typedef struct stackframe
{
	u32 gs;
	u32 fs;
	u32 es;
	u32 ds;
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 kernel_esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;
	u32 retaddr;
	u32 eip;
	u32 cs;
	u32 eflags;
	u32 esp;
	u32 ss;
}STACK_FRAME;

typedef struct proc
{
	STACK_FRAME s_regs;

	u16 ldt_sel;
	DESCRIPTOR s_ldts[LDT_SIZE];

	int ticks;
	int priority;

	u32 pid;
	char name[16];

	int flags;
	MESSAGE *p_msg;
	int recvfrom;
	int sendto;
	int has_int_msg;
	struct proc *q_sending;
	struct proc *next_sending;
	int parent;
	int exit_status;

	int nr_tty;

	struct file_desc *filp[NR_FILES];
}PROCESS;

typedef struct task
{
	task_f initial_eip;
	int stacksize;
	char name[32];
}TASK;

#define proc2pid(x) (x-proc_table)

#define NR_TASKS 5
#define NR_PROCS 32
#define NR_NATIVE_PROCS 4
#define FIRST_PROC	proc_table[0]
#define LAST_PROC	proc_table[NR_TASKS + NR_PROCS - 1]

#define STACK_DEFAULT_SIZE 0x8000
#define STACK_SIZE_TTY   0x8000
#define STACK_SIZE_SYS   0x8000
#define STACK_SIZE_HD	 0x8000
#define STACK_SIZE_FS	 0x8000
#define STACK_SIZE_MM    0x8000
#define STACK_SIZE_INIT  0x8000
#define STACK_SIZE_TESTA 0x8000
#define STACK_SIZE_TESTB 0x8000
#define STACK_SIZE_TESTC 0x8000
#define STACK_SIZE_TOTAL (STACK_DEFAULT_SIZE * 9)

enum msgtype {
	HARD_INT = 1,

	/* SYS task */
	GET_TICKS,GET_PID,GET_RTC_TIME,

	OPEN,CLOSE,READ,WRITE,LSEEK,STAT,UNLINK,

	SUSPEND_PROC, RESUME_PROC, 

	EXEC,WAIT,

	FORK, EXIT,

	SYSCALL_RET,
	/* message type for drivers */
	DEV_OPEN = 1001,
	DEV_CLOSE,
	DEV_READ,
	DEV_WRITE,
	DEV_IOCTL
};
#define FD      u.m3.m3i1
#define PATHNAME    u.m3.m3p1
#define FLAGS       u.m3.m3i1
#define NAME_LEN    u.m3.m3i2
#define BUF_LEN     u.m3.m3i3
#define	CNT		u.m3.m3i2
#define	REQUEST		u.m3.m3i2
#define	PROC_NR		u.m3.m3i3
#define	DEVICE		u.m3.m3i4
#define	POSITION	u.m3.m3l1
#define	BUF		u.m3.m3p2
#define OFFSET      u.m3.m3i2
#define WHENCE      u.m3.m3i3
#define RETVAL u.m3.m3i1
#define PID     u.m3.m3i2
#define STATUS      u.m3.m3i1
#define RETVAL      u.m3.m3i1

extern PROCESS proc_table[NR_TASKS+ NR_PROCS];
extern int k_reenter;

void init_proc();
void inform_int(int task_nr);
void dump_proc(struct proc* p);
void dump_msg(const char * title, MESSAGE* m);
void reset_msg(MESSAGE *p);
void *va2la(int pid,void *va);
void reset_msg(MESSAGE *p);
int send_recv(int function,int src_dest,MESSAGE *msg);
int sys_sendrec(int function,int src_dest,MESSAGE* m,struct proc*p);
int ldt_seg_linear(struct proc *p,int idx);

#endif

