#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#define BLACK 0x0
#define WHITE 0x7
#define RED 0x4
#define GREEN 0x2
#define BLUE 0x1
#define FLASH 0x80
#define BRIGHT 0x08
#define MAKE_COLOR(x,y) ((x<<4)  | y)

#define V_MEM_BASE 0xb8000
#define V_MEM_SIZE 0x8000
#define NR_CONSOLES 3

typedef struct console
{
	u32 current_start_addr;
	u32 original_addr;
	u32 size;
	u32 cursor;
	int is_full;
}CONSOLE;

#define SCR_UP 1
#define SCR_DN -1
#define SCREEN_SIZE (80*25)
#define SCREEN_WIDTH  80
#define DEFAULT_CHAR_COLOR (MAKE_COLOR(BLACK,WHITE))
#define GRAY_CHAR (MAKE_COLOR(BLACK,BLACK)|BRIGHT)
#define RED_CHAR (MAKE_COLOR(BLUE,RED) | BRIGHT)


extern int disp_pos;
extern int nr_current_console;

extern void scroll_screen(CONSOLE* p_con,int direction);
extern void select_console(int nr_console);
extern void init_screen(TTY* p_tty);
extern int is_current_console(CONSOLE* p_con);
extern void out_char(CONSOLE* p_con,char ch);

#endif
