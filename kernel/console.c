#include "port.h"
#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "proto.h"
#include "keyboard.h"

CONSOLE console_table[NR_CONSOLES];
int disp_pos;
int nr_current_console;

static void set_cursor(u32 position);
static void set_video_start_addr(u32 addr);
static void flush(CONSOLE* p_con);
static void w_copy(unsigned int dst, const unsigned int src, int size);
static void clear_screen(int pos, int len);

void scroll_screen(CONSOLE* p_con,int direction)
{
	if(direction == SCR_UP)
	{
		if(((int)(p_con->current_start_addr)-SCREEN_WIDTH) > (int)(p_con->original_addr))
			p_con->current_start_addr-=SCREEN_WIDTH;
	}
	else if(direction == SCR_DN)
	{
		if(p_con->current_start_addr+SCREEN_SIZE < p_con->original_addr+p_con->size )
			p_con->current_start_addr+=SCREEN_WIDTH;
	}
	else
	{
		set_video_start_addr(p_con->current_start_addr);
		set_cursor(p_con->cursor);
	}
}

int is_current_console(CONSOLE* p_con)
{
	return (p_con ==  console_table+ nr_current_console);
}

void out_char(CONSOLE* p_con,char ch)
{
	u8* p_vmem=(u8*)(V_MEM_BASE +p_con->cursor*2);
	int cursor_x =(p_con->cursor - p_con->original_addr) % SCREEN_WIDTH;
	int cursor_y = (p_con->cursor - p_con->original_addr) / SCREEN_WIDTH;
	switch(ch)
	{
		case '\n':
			p_con->cursor=p_con->original_addr+SCREEN_WIDTH*(cursor_y+1);
			break;
		case '\b':
			if(p_con->cursor> p_con->original_addr)
			{
				p_con->cursor--;
				*(p_vmem-2)=' ';
				*(p_vmem-1)=DEFAULT_CHAR_COLOR;
			}
			break;
		default:
			*p_vmem++=ch;
			*p_vmem++=DEFAULT_CHAR_COLOR;
			p_con->cursor++;
			break;
	}
	
	if (p_con->cursor - p_con->original_addr >= p_con->size) {
		cursor_x = (p_con->cursor - p_con->original_addr) % SCREEN_WIDTH;
		cursor_y = (p_con->cursor - p_con->original_addr) / SCREEN_WIDTH;
		int cp_original_addr = p_con->original_addr + (cursor_y + 1) * SCREEN_WIDTH - SCREEN_SIZE;
		w_copy(p_con->original_addr, cp_original_addr, SCREEN_SIZE - SCREEN_WIDTH);
		p_con->current_start_addr = p_con->original_addr;
		p_con->cursor = p_con->original_addr + (SCREEN_SIZE - SCREEN_WIDTH) + cursor_x;
		clear_screen(p_con->cursor, SCREEN_WIDTH);
		if (!p_con->is_full)
			p_con->is_full = 1;
	}

	while(p_con->cursor>=p_con->current_start_addr+SCREEN_SIZE || p_con->cursor < p_con->current_start_addr)
	{
		scroll_screen(p_con,SCR_DN);
		clear_screen(p_con->cursor,SCREEN_WIDTH);
	}
	if(is_current_console(p_con))
		flush(p_con);
}
static void clear_screen(int pos, int len) 
{          
	u8 * pch = (u8*)(V_MEM_BASE + pos * 2);
	while (--len >= 0) 
	{
		*pch++ = ' ';
		*pch++ = DEFAULT_CHAR_COLOR;
	}
}   
static void w_copy(unsigned int dst, const unsigned int src, int size)
{           
	memcpy((void*)(V_MEM_BASE + (dst << 1)),
			(void*)(V_MEM_BASE + (src << 1)),
			size << 1);
}
    


static void flush(CONSOLE* p_con)
{
	set_cursor(p_con->cursor);
	set_video_start_addr(p_con->current_start_addr);
}


void set_cursor(u32 position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG,CURSOR_H);
	out_byte(CRTC_DATA_REG,(position>>8)&0xff);
	out_byte(CRTC_ADDR_REG,CURSOR_L);
	out_byte(CRTC_DATA_REG,position & 0xff);
	enable_int();
}


void init_screen(TTY* p_tty)
{
	int nr_tty=p_tty-tty_table;
	p_tty->p_console=console_table+nr_tty;
	int v_mem_size=V_MEM_SIZE>>1;
	int con_v_mem_size=v_mem_size/NR_CONSOLES;
	p_tty->p_console->original_addr = nr_tty*con_v_mem_size;
	p_tty->p_console->size = con_v_mem_size/SCREEN_WIDTH *SCREEN_WIDTH;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;
	p_tty->p_console->cursor=p_tty->p_console->original_addr;
	if(nr_tty ==0)
	{
		p_tty->p_console->cursor=disp_pos/2;
		disp_pos=0;
	}
	else
	{
		out_char(p_tty->p_console,nr_tty+'0');
		out_char(p_tty->p_console,'#');
	}

	set_cursor(p_tty->p_console->cursor);
}

void select_console(int nr_console)
{
	if((nr_console<0) ||(nr_console>=NR_CONSOLES))
	{
		return;
	}
	nr_current_console=nr_console;
	set_cursor(console_table[nr_current_console].cursor);
	set_video_start_addr(console_table[nr_current_console].current_start_addr);
}


static void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG,START_ADDR_H);
	out_byte(CRTC_DATA_REG,(addr>>8)&0xff);
	out_byte(CRTC_ADDR_REG,START_ADDR_L);
	out_byte(CRTC_DATA_REG,addr&0xff);
	enable_int();
}
