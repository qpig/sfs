#ifndef	 _CONST_H_
#define	 _CONST_H_

#define	PUBLIC
#define	PRIVAT	static

#define STR_DEFAULT_LEN 1024

#define TRUE 1
#define FALSE 0

/*privilege*/
#define PRIVILEGE_KERNEL 0
#define PRIVILEGE_TASK 1
#define PRIVILEGE_USER 3

/* RPL */
#define RPL_KERNEL SA_RPL0
#define RPL_TASK	SA_RPL1
#define RPL_USER	SA_RPL3

/*keyboard*/
#define LED_CODE 0xed
#define KB_ACK 0xfa

/*error*/
#define ASSERT
#ifdef ASSERT
void assertion_failure(char *exp,char *file,char *base_file,int line);
#define assert(exp) {if(exp);\
	else assertion_failure(#exp,__FILE__,__BASE_FILE__,__LINE__);}
#else
#define assert(exp)
#endif

/*tasks*/

/*ipc*/

/*magic chars*/
#define MAG_CH_PANIC '\002'
#define MAG_CH_ASSERT '\003'



/* Hard Drive */

/* device numbers of hard disk */


/**
 * @def MAX_PRIM_DEV
 * Defines the max minor number of the primary partitions.
 * If there are 2 disks, prim_dev ranges in hd[0-9], this macro will
 * equals 9.
 */


#define	is_special(m)	((((m) & I_TYPE_MASK) == I_BLOCK_SPECIAL) ||	\
			 (((m) & I_TYPE_MASK) == I_CHAR_SPECIAL))

#define	NR_DEFAULT_FILE_SECTS	2048 /* 2048 * 512 = 1MB */

#endif
