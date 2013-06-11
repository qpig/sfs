#ifndef _I8259_
#define _I8259_

#define INT_M_CTL		0x20
#define INT_M_CTLMASK	0x21
#define INT_S_CTL		0xA0
#define INT_S_CTLMASK	0xA1

#define NR_IRQ 16
#define CLOCK_IRQ	0
#define KEYBOARD_IRQ 1
#define CASCADE_IRQ	2
#define ETHER_IRQ	3
#define SECONDARY_IRQ	3
#define RS232_IRQ	4
#define XT_WINI_IRQ	5
#define FLOPPY_IRQ	6
#define	PRINTER_IRQ	7
#define AT_WINI_IRQ	14

void init_8259A();
void put_irq_handler(int irq,irq_handler handler);

#endif
