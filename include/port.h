#ifndef _PORT_H_
#define _PORT_H_
/*8259A*/

/*clock.c*/
#define TIMER0_REG	0x40
#define TIMER_MODE_REG	0x43


/*tty.c*/
#define CRTC_ADDR_REG 0x3d4
#define CRTC_DATA_REG 0x3d5
#define START_ADDR_H  0xc
#define START_ADDR_L  0xd
#define CURSOR_H 0xe
#define CURSOR_L 0xf

/*keyboard*/
#define KB_DATA 0x60
#define KB_CMD 0x64


#endif
