#include "port.h"
#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "fs.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "proto.h"
#include "time.h"
#include "lib/stdio.h"
#include "lib/fcntl.h"

int kernel_main()
{
	ticks=0;

	disp_str("-----kernel_main begins ----\n");
	
	init_proc();

	init_clock();

	restart();

	while(1)
	{}
}


void TestA()
{
	while(1)
	{
		//printf("%d",get_ticks());
		//int fd = open( "/dev_ttydf",O_CREAT );
		//printk(" fd: %d\n", fd );
		milli_delay(1000);
	}
}

void shell()
{
	//char tty_name[] = "/dev_tty1";
	//int fd_stdin = open( tty_name, O_RDWR );
	//assert( fd_stdin == 0 );
	//int fd_stdout = open( tty_name, O_RDWR );
	//assert( fd_stdout == 1 );
	char rdbuf[70]={0};
	
	while(1)
	{
		printf("$");
		int r = read( 0, rdbuf, 70 );
		rdbuf[r] = 0;

		int argc = 0;
		char *argv[PROC_STACK]={0};
		char *p = rdbuf;
		char *s;
		int word = 0;
		char ch;
		do{
			ch = *p;
			if( ch != ' ' && ch != 0 && !word )
			{
				s = p;
				word = 1;
			}
			if( (ch == ' ' || ch == 0) && word )
			{
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}
			p++;
		}while( ch );
		argv[argc] = 0;

		int fd = open( argv[0], O_RDWR );
		printf("fd:%d\n",fd);
		close(fd);
		if( fd == -1 )
		{
			if( rdbuf[0] )
				printf("{%s}\n",rdbuf);
		}
		else
		{
			int pid = fork();
			if( pid != 0 )
			{
				int s;
				wait(&s);
				printf("pid:%d,exit:%d\n", pid,s);
			}
			else
			{
				execv( argv[0], argv );
			}
		}
	}
	spin("Init");
}

void TestC()
{
	while(1)
	{
		//printf("c");
		milli_delay(1000);
	}
}
void Init()
{
	char tty_name[] = "/dev_tty1";
	int fd_stdin = open( tty_name, O_RDWR );
	assert( fd_stdin == 0 );
	int fd_stdout = open( tty_name, O_RDWR );
	assert( fd_stdout == 1 );
	//int fd_echo = open("/echo", O_RDWR );
	//read( fd_echo, buf, 512 );
	//int fd_dir = open( "/.", O_RDWR );
	//read( fd_dir, buf, SECTOR_SIZE );
	//int i= sizeof(struct d_btree_node);
	//printf( "%d.", i);

	int pid = fork();
	if( pid != 0 )
	{
		printf( "parent is running child pid: %d\n", pid );
	}
	else
	{
		printf( "child is runing pid: %d\n", getpid() );
		//close(fd_stdin);
		//close(fd_stdout);
		shell();
		assert(0);
	}
	while(1)
	{
		int s;
		int child = wait( &s );
		printf( "child (%d) exited with status: %d.\n", child, s );
	}
	assert(0);
}

void TestB()
{
	int fd_stdin  = open("/dev_tty1", O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open("/dev_tty1", O_RDWR);
	assert(fd_stdout == 1);

	printf("TestB() is running ...\n");
	spin("TestB");

	/*
	while(1)
	{
		printf("$");
		int r = read( fd_stdin, rdbuf, 70 );
		rdbuf[r] = 0;
		if( r > 0 )
			printf("{%s}\n",rdbuf );
	}
	*/
	/* can't fork
	int pid = fork();
	if (pid != 0) { 
		printf("parent is running, child pid:%d\n", pid);
		int s;
		int child = wait(&s);
		printf("child (%d) exited with status: %d.\n", child, s );
	}
	else {  
		printf("child is running, pid:%d\n", getpid());
		exit(123);
	}
	while(1)
	{
		milli_delay(1000);
	}
	*/
	
}
