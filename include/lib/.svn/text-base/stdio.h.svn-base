#ifndef _STDIO_
#define _STDIO_

#include "stddef.h"
#include "stdarg.h"

extern int vsprintf(char* buf,const char* fmt,va_list args);
extern int sprintf(char *buf, const char *fmt, ...);
extern int printk(const char *fmt, ...);
extern int printf( const char *fmt, ... );

extern void spin(char *func_name);
extern void panic(const char *fmt,...);
extern void  assertion_failure(char *exp,char *file,char *base_file,int line);

#endif
