/*目前execve不能解析elf程序文件，因此filename文件必须是bin格式程序,
	并且，start_addr假定为0x0，end_data为文件大小,
	start_stack为0xc0000000(VM_START),
	end_code和brk暂时不支持
*/
#include<linux/fs.h>
#include<memory.h>
#include<sys/stat.h>
extern int permission(m_inode * inode,int mask);

void invalid_user_page()
{
	int startAddr;
	u32 *pde=(u32 *)(current->cr3+VM_START);
	for(startAddr=0;startAddr<VM_START;startAddr+=1024*PAGE_SIZE){
		pde[startAddr>>22]=0;
	}
	en_page(current->cr3);
}


int sys_execve(const char * filename, char ** argv, char ** envp)
{
	m_inode *execfile;	
	DEBUG("exec:%s\n",filename);
	if( !(execfile=namei(filename)) )
		return -1;

	if( !permission(execfile, 1) )//检查执行权限
		return -1;
	
	if(!S_ISREG(execfile->i_mode))//不是可执行文件
		return -1;
	
	
	current->executable=execfile;
	current->start_code=0x2000;
	current->end_data=execfile->i_size+current->start_code;
	current->start_stack=0x4000;
	invalid_user_page();
	asm("push %0\n"	//ess
		"push %3\n"	//esp
		"push %1\n"	//cs
		"push %2\n"	//eip
		"mov %0,%%eax\n"
		"mov %%ax,%%ds\n"
		"mov %%ax,%%es\n"
		"mov %%ax,%%fs\n"
		"lret"
		::"i"(SELECTOR(USER_DATA,PL_USER)),
		"i"(SELECTOR(USER_CODE,PL_USER)),
		"g"(current->start_code),
		"g"(current->start_stack));
}

	



