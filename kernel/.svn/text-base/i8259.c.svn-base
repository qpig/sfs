#include "port.h"
#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proto.h"
#include "asm/io.h"
#include "i8259.h"
#include "lib/stdio.h"

irq_handler irq_table[NR_IRQ];
void spurious_irq(int irq);

void init_8259A()
{
	outb_p( 0x11, INT_M_CTL);

	outb_p( 0x11, INT_S_CTL );

	outb_p( INT_VECTOR_IRQ0, INT_M_CTLMASK );

	outb_p( INT_VECTOR_IRQ8, INT_S_CTLMASK );

	outb_p( 0x4, INT_M_CTLMASK );

	outb_p( 0x2, INT_S_CTLMASK );

	outb_p( 0x1, INT_M_CTLMASK );

	outb_p( 0x1, INT_S_CTLMASK );

	outb_p( 0xff, INT_M_CTLMASK );

	outb_p( 0xff, INT_S_CTLMASK );

	int i;
	for(i=0;i<NR_IRQ;i++)
	{
		irq_table[i]=spurious_irq;
	}
}


void spurious_irq(int irq)
{
	printk("suprious_irq: %d \n", irq );
}

void put_irq_handler(int irq,irq_handler handler)
{
	disable_irq(irq);
	irq_table[irq]=handler;
}
