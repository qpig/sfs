#include "fs.h"

#include "elf.h"

u8 *        mmbuf       = (u8*)0x700000;
const int   MMBUF_SIZE  = 0x100000;

int do_exec( MESSAGE *msg )
{
	int name_len = msg->NAME_LEN;
	int src = msg->source;
	assert( name_len < MAX_PATH );

	char pathname[MAX_PATH];
	memcpy( (void *)va2la(TASK_MM,pathname), (void *)va2la(src,msg->PATHNAME), name_len );
	pathname[name_len] = 0;

	int fd = open(pathname, O_RDWR);
	if( fd == -1 )
		return -1;
	int size = SECTOR_SIZE * 64;
	read( fd, mmbuf, size ); 
	close( fd );
	
	Elf32_Ehdr *elf_ehdr = (Elf32_Ehdr *)mmbuf;
	int i;
	for( i=0; i< elf_ehdr->e_phnum; i++ )
	{
		Elf32_Phdr *elf_phdr = (Elf32_Phdr *)(mmbuf + elf_ehdr->e_phoff + i*elf_ehdr->e_phentsize );
		if( elf_phdr->p_type == PT_LOAD )
		{
			assert( elf_phdr->p_vaddr + elf_phdr->p_memsz < PROC_IMAGE_SIZE_DEFAULT );
			memcpy( (void *)va2la( src, (void *)elf_phdr->p_vaddr), (void *)va2la( TASK_MM, mmbuf + elf_phdr->p_offset), elf_phdr->p_filesz );
		}
	}

	int stack_len = msg->BUF_LEN;
	char stackcopy[PROC_STACK];
	memcpy( (void *)va2la( TASK_MM, stackcopy), (void *)va2la( src, msg->BUF), stack_len );
	u8 *stack = (u8 *)(PROC_IMAGE_SIZE_DEFAULT - PROC_STACK );
	int delta = (int)stack - (int)msg->BUF;
	int argc = 0;
	if( stack_len )
	{
		char **q = (char **)stackcopy;
		for( ; *q!=0; q++,argc++ )
			*q += delta;
	}

	memcpy( (void *)va2la( src, stack ), (void *)va2la( TASK_MM, stackcopy ), stack_len );
	proc_table[src].s_regs.ecx = argc;
	proc_table[src].s_regs.eax = (u32)stack;
	proc_table[src].s_regs.eip = elf_ehdr->e_entry;
	proc_table[src].s_regs.esp = PROC_IMAGE_SIZE_DEFAULT - PROC_STACK;

	strcpy( proc_table[src].name, pathname );

	return 0;
}
