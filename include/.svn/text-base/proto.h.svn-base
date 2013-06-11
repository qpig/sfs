#ifndef _PROTO_H_
#define _PROTO_H_

void task_hd();
void task_fs();
void task_sys();
void task_mm();

extern void Init();
extern void TestA();
extern void TestB();
extern void TestC();

/*keyboard.c*/
void init_keyboard();
void keyboard_read();

/* kliba.s */
void disable_int();
void enable_int();
void out_byte(u16 port,u8 value);
u8 in_byte(u16 port);
void disp_str(char* info);
void disp_color_str(char* info,int color);
void disable_irq(int irq);
void enable_irq(int irq);

/* kernel.s */
void restart();

/*syscall.S*/
extern void sys_call();
extern int sendrec(int function,int src_dest,MESSAGE *m);
extern int printx(char *str);

#endif
