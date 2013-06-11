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
#include "keymap.h"
#include "i8259.h"

static KB_INPUT kb_in;
static int code_with_E0=0;
static int shift_l;
static int shift_r;
static int alt_l;
static int alt_r;
static int ctrl_l;
static int ctrl_r;
static int caps_lock;
static int num_lock;
static int scroll_lock;
static int column;

static u8 get_byte_from_kbuf();
static void kb_wait();
static void kb_ack();
static void set_leds();

void keyboard_handler(int irq)
{
	u8 scan_code=in_byte(0x60);
	
	if(kb_in.count< KB_IN_BYTES)
	{
		*(kb_in.p_head++)=scan_code;
		if(kb_in.p_head == kb_in.buf+KB_IN_BYTES)
		{
			kb_in.p_head=kb_in.buf;
		}
		kb_in.count++;
	}
	key_pressed = 1;
}

void init_keyboard()
{
	kb_in.count=0;
	kb_in.p_head=kb_in.p_tail=kb_in.buf;
	shift_l=shift_r=0;
	alt_l=alt_r=0;
	ctrl_l=ctrl_r=0;
	caps_lock=0;
	num_lock=1;
	scroll_lock=0;
	set_leds();
	put_irq_handler(KEYBOARD_IRQ,keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
}

void keyboard_read(TTY* p_tty)
{
	u8 scan_code;
	int make;
	u32 key=0;
	u32* keyrow=0;

	if(kb_in.count > 0)
	{
		code_with_E0=0;
		disable_int();
		scan_code=get_byte_from_kbuf();

		if(scan_code == 0xe1)
		{
			int i;
			u8 pausebreak_scode[]={0xE1,0x1D,0x45,0xE1,0x9d,0xc5};
			int is_pausebreak=1;
			for(i=1;i<6;i++)
			{
				if(get_byte_from_kbuf()!=pausebreak_scode[i])
				{
					is_pausebreak=0;
					break;
				}
			}
			if(is_pausebreak)
			{
				key=PAUSEBREAK;
			}
		}
		else if(scan_code == 0xe0)
		{
			scan_code=get_byte_from_kbuf();
			if(scan_code== 0x2a)
				if(get_byte_from_kbuf() == 0xe0)
					if(get_byte_from_kbuf() == 0x37)
					{
						key=PRINTSCREEN;
						make=1;
					}
			if(scan_code == 0xb7)
				if(get_byte_from_kbuf() == 0xe0)
					if(get_byte_from_kbuf() == 0xaa)
					{
						key=PRINTSCREEN;
						make=0;
					}
			if(key==0)
				code_with_E0=1;
		}
		if((key != PAUSEBREAK) && (key != PRINTSCREEN))
		{
			make=(scan_code & FLAG_BREAK ? FALSE : TRUE);
			keyrow=&keymap[(scan_code & 0x7f)* MAP_COLS];
			column=0;

			int caps=shift_l||shift_r;
			if(caps_lock)
			{
				if((keyrow[0] >= 'a') && (keyrow[0] <= 'z'))
				{
					caps=!caps;
				}
			}
			if(caps)
			{
				column=1;
			}
			if(code_with_E0)
			{
				column=2;
				code_with_E0=0;
			}
			key=keyrow[column];

			switch(key)
			{
				case SHIFT_L:
					shift_l=make;
					break;
				case SHIFT_R:
					shift_r=make;
					break;
				case CTRL_L:
					ctrl_l=make;
					break;
				case CTRL_R:
					ctrl_r=make;
					break;
				case ALT_L:
					alt_l=make;
					break;
				case ALT_R:
					alt_r=make;
					break;
				case CAPS_LOCK:
					if(make)
					{
						caps_lock = !caps_lock;
						set_leds();
					}
					break;
				case NUM_LOCK:
					if(make)
					{
						num_lock=!num_lock;
						set_leds();
					}
					break;
				case SCROLL_LOCK:
					if(make)
					{
						scroll_lock=!scroll_lock;
						set_leds();
					}
					break;
				default:
					break;
			}
			if(make == 1)
			{
				int pad=0;
				if((key>=PAD_SLASH) && (key<=PAD_9))
				{
					pad=1;
					switch(key)
					{
						case PAD_SLASH:
							key='/';
							break;
						case PAD_STAR:
							key='*';
							break;
						case PAD_MINUS:
							key='-';
							break;
						case PAD_PLUS:
							key='+';
							break;
						case PAD_ENTER:
							key=ENTER;
							break;
						default:
							if(num_lock && (key>=PAD_0) && (key<=PAD_9))
							{
								key=key-PAD_0+'0';
							}
							else if(num_lock && key == PAD_DOT)
							{
								key='.';
							}
							else
							{
								switch(key)
								{
									case PAD_HOME:
										key=HOME;
										break;
									case PAD_END:
										key=END;
										break;
									case PAD_PAGEUP:
										key=PAGEUP;
										break;
									case PAD_PAGEDOWN:
										key=PAGEDOWN;
										break;
									case PAD_INS:
										key=INSERT;
										break;
									case PAD_UP:
										key=UP;
										break;
									case PAD_DOWN:
										key=DOWN;
										break;
									case PAD_LEFT:
										key=LEFT;
										break;
									case PAD_RIGHT:
										key=RIGHT;
										break;
									case PAD_DOT:
										key=DELETE;
										break;
									default:
										break;
								}
							}
							break;
					}
				}
				key |= shift_l ? FLAG_SHIFT_L :0;
				key |= shift_r ? FLAG_SHIFT_R :0;
				key |= ctrl_l ? FLAG_CTRL_L :0;
				key |= ctrl_r ? FLAG_CTRL_R :0;
				key |= alt_l ? FLAG_ALT_L :0;
				key |= alt_r ? FLAG_ALT_R :0;
				key |=pad ? FLAG_PAD :0;

				in_process(p_tty,key);
			}
		}
	}
}


static u8 get_byte_from_kbuf()
{
	u8 scan_code;
	while(kb_in.count <=0)
	{}

	disable_int();
	scan_code=*(kb_in.p_tail++);
	if(kb_in.p_tail == kb_in.buf+KB_IN_BYTES)
	{
		kb_in.p_tail=kb_in.buf;
	}
	kb_in.count--;
	enable_int();
	return scan_code;
}

static void kb_wait()
{
	u8 kb_stat;
	do
	{
		kb_stat = in_byte(KB_CMD);
	}while(kb_stat&0x02);
}

static void kb_ack()
{
	u8 kb_read;
	do
	{
		kb_read=in_byte(KB_DATA);
	}while(kb_read!=KB_ACK);
}

static void set_leds()
{
	u8 leds=(caps_lock<<2)|(num_lock<<1) |scroll_lock;
	kb_wait();
	out_byte(KB_DATA,LED_CODE);
	kb_ack();
	kb_wait();
	out_byte(KB_DATA,leds);
	kb_ack();
}
