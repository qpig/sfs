#ifndef _TIME_
#define _TIME_

#define RATE_GENERATOR	0x34
#define TIMER_FREG  1193182L
#define HZ		100
#define MAX_TICKS 0x7FFFABCD

extern int ticks;
extern int get_ticks();
extern void clock_handler(int irq);
extern void milli_delay(int milli_sec);
extern void init_clock();

#endif
