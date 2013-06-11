#include "port.h"
#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "proto.h"
#include "i8259.h"
#include "lib/stdio.h"


TSS tss;
u8 gdt_ptr[6];
DESCRIPTOR gdt[GDT_SIZE];
u8	idt_ptr[6];
GATE idt[IDT_SIZE];

/* 本文件内函数声明 */
static void init_idt_desc(unsigned char vector, u8 desc_type,
			   int_handler handler, unsigned char privilege);

/* 中断处理函数 */
void	divide_error();
void	single_step_exception();
void	nmi();
void	breakpoint_exception();
void	overflow();
void	bounds_check();
void	inval_opcode();
void	copr_not_available();
void	double_fault();
void	copr_seg_overrun();
void	inval_tss();
void	segment_not_present();
void	stack_exception();
void	general_protection();
void	page_fault();
void	copr_error();
void    hwint00();
void    hwint01();
void    hwint02();
void    hwint03();
void    hwint04();
void    hwint05();
void    hwint06();
void    hwint07();
void    hwint08();
void    hwint09();
void    hwint10();
void    hwint11();
void    hwint12();
void    hwint13();
void    hwint14();
void    hwint15();

void init_protect()
{
	init_8259A();
	init_idt_desc(INT_VECTOR_SYS_CALL,DA_386IGate,
			sys_call,PRIVILEGE_USER);
	// 全部初始化成中断门(没有陷阱门)
	init_idt_desc(INT_VECTOR_DIVIDE,	DA_386IGate,
		      divide_error,		PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_DEBUG,		DA_386IGate,
		      single_step_exception,	PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_NMI,		DA_386IGate,
		      nmi,			PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_BREAKPOINT,	DA_386IGate,
		      breakpoint_exception,	PRIVILEGE_USER);
	init_idt_desc(INT_VECTOR_OVERFLOW,	DA_386IGate,
		      overflow,			PRIVILEGE_USER);
	init_idt_desc(INT_VECTOR_BOUNDS,	DA_386IGate,
		      bounds_check,		PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_INVAL_OP,	DA_386IGate,
		      inval_opcode,		PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_COPROC_NOT,	DA_386IGate,
		      copr_not_available,	PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_DOUBLE_FAULT,	DA_386IGate,
		      double_fault,		PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_COPROC_SEG,	DA_386IGate,
		      copr_seg_overrun,		PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_INVAL_TSS,	DA_386IGate,
		      inval_tss,		PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_SEG_NOT,	DA_386IGate,
		      segment_not_present,	PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_STACK_FAULT,	DA_386IGate,
		      stack_exception,		PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_PROTECTION,	DA_386IGate,
		      general_protection,	PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_PAGE_FAULT,	DA_386IGate,
		      page_fault,		PRIVILEGE_KERNEL);
	init_idt_desc(INT_VECTOR_COPROC_ERR,	DA_386IGate,
		      copr_error,		PRIVILEGE_KERNEL);

		init_idt_desc(INT_VECTOR_IRQ0 + 0,      DA_386IGate,
                      hwint00,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ0 + 1,      DA_386IGate,
                      hwint01,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ0 + 2,      DA_386IGate,
                      hwint02,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ0 + 3,      DA_386IGate,
                      hwint03,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ0 + 4,      DA_386IGate,
                      hwint04,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ0 + 5,      DA_386IGate,
                      hwint05,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ0 + 6,      DA_386IGate,
                      hwint06,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ0 + 7,      DA_386IGate,
                      hwint07,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ8 + 0,      DA_386IGate,
                      hwint08,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ8 + 1,      DA_386IGate,
                      hwint09,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ8 + 2,      DA_386IGate,
                      hwint10,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ8 + 3,      DA_386IGate,
                      hwint11,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ8 + 4,      DA_386IGate,
                      hwint12,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ8 + 5,      DA_386IGate,
                      hwint13,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ8 + 6,      DA_386IGate,
                      hwint14,                  PRIVILEGE_KERNEL);
        init_idt_desc(INT_VECTOR_IRQ8 + 7,      DA_386IGate,
                      hwint15,                  PRIVILEGE_KERNEL);

	int i;
	PROCESS* p_proc=proc_table;
	u16 selector_ldt=INDEX_LDT_FIRST<<3;
	for(i=0;i<NR_TASKS+NR_PROCS;i++)
	{
		memset( p_proc, 0, sizeof(struct proc) );
		p_proc->ldt_sel = selector_ldt;
		init_descriptor(&gdt[selector_ldt>>3],
				vir2phys(seg2phys(SELECTOR_KERNEL_DS),proc_table[i].s_ldts),
				LDT_SIZE*sizeof(DESCRIPTOR)-1,DA_LDT);
		p_proc++;
		selector_ldt+=1<<3;
	}
	memset(&tss,0,sizeof(tss));
	tss.ss0=SELECTOR_KERNEL_DS;
	init_descriptor(&gdt[INDEX_TSS],
			vir2phys(seg2phys(SELECTOR_KERNEL_DS),&tss),
			sizeof(tss)-1,DA_386TSS);
	tss.iobase=sizeof(tss);
}


u32 seg2phys(u16 seg)
{
	DESCRIPTOR* p_dest=&gdt[seg >> 3];
	return (p_dest->base_high << 24 | p_dest->base_mid <<16 | p_dest->base_low);
	
}

static void init_idt_desc(unsigned char vector, u8 desc_type,
			  int_handler handler, unsigned char privilege)
{
	GATE *	p_gate	= &idt[vector];
	u32	base	= (u32)handler;
	p_gate->offset_low	= base & 0xFFFF;
	p_gate->selector	= SELECTOR_KERNEL_CS;
	p_gate->dcount		= 0;
	p_gate->attr		= desc_type | (privilege << 5);
	p_gate->offset_high	= (base >> 16) & 0xFFFF;
}

void init_descriptor(DESCRIPTOR* p_desc,u32 base,u32 limit,u16 attribute)
{
	p_desc->limit_low=limit&0x0ffff;
	p_desc->base_low=base & 0x0ffff;
	p_desc->base_mid=(base>>16) & 0xff;
	p_desc->attr1 =attribute & 0xff;
	p_desc->limit_high_attr2=((limit>>16) & 0xf) |((attribute >>8) & 0xf0);
	p_desc->base_high=(base>>24) & 0xff;
}

void exception_handler(int vec_no,int err_code,int eip,int cs,int eflags)
{
	int i;
	int text_color = 0x74; /* 灰底红字 */

	char * err_msg[] = {"#DE Divide Error",
			    "#DB RESERVED",
			    "--  NMI Interrupt",
			    "#BP Breakpoint",
			    "#OF Overflow",
			    "#BR BOUND Range Exceeded",
			    "#UD Invalid Opcode (Undefined Opcode)",
			    "#NM Device Not Available (No Math Coprocessor)",
			    "#DF Double Fault",
			    "    Coprocessor Segment Overrun (reserved)",
			    "#TS Invalid TSS",
			    "#NP Segment Not Present",
			    "#SS Stack-Segment Fault",
			    "#GP General Protection",
			    "#PF Page Fault",
			    "--  (Intel reserved. Do not use.)",
			    "#MF x87 FPU Floating-Point Error (Math Fault)",
			    "#AC Alignment Check",
			    "#MC Machine Check",
			    "#XF SIMD Floating-Point Exception"
	};

	disp_pos = 0;
	for(i=0;i<80*15;i++){
		disp_str(" ");
	}
	//disp_pos = 0;

	disp_color_str("Exception! --> ", text_color);
	disp_color_str(err_msg[vec_no], text_color);
	disp_color_str("\n\n", text_color);
	disp_color_str("EFLAGS:", text_color);
	printk( "%x", eflags );
	disp_color_str("CS:", text_color);
	printk( "%x", cs );
	disp_color_str("EIP:", text_color);
	printk( "%x", eip );

	if(err_code != 0xFFFFFFFF){
		disp_color_str("Error code:", text_color);
		printk( "%x", err_code );
	}
}

