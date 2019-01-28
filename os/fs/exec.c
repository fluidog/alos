/*目前execve不能解析elf程序文件，因此filename文件必须是bin格式程序,
	并且，start_addr假定为0x0，end_data为文件大小,
	start_stack为0xc0000000(VM_START),
	end_code和brk暂时不支持
*/
#include<linux/fs.h>
#include<memory.h>
#include<sys/stat.h>
extern int permission(m_inode * inode,int mask);

int sys_execve(const char * filename, char ** argv, char ** envp,
			long fs, long es, long ds,long eip, long cs, long eflags, long esp, long ss)
{
	m_inode *execfile;	
	if( !(execfile=namei(filename)) )
		return -1;
	if( !permission(execfile, 1) )//检查执行权限
		return -1;
	if(!S_ISREG(execfile->i_mode))//不是可执行文件
		return -1;

	if(current->executable)//第一个进程刚开始没有executable
		iput(current->executable);
	current->executable=execfile;
	
	//之后增加解析elf格式之后，根据elf来设置
	current->start_code=0x2000;
	current->end_data=execfile->i_size+current->start_code;
	current->start_stack=0x6000;

	//重置用户空间
	free_page_table();
	
	eip=current->start_code;
	esp=current->start_stack;
	//cs,ss,ds,es,fs本可以不设置,但是考虑到第一个进程是从内核进入此函数的，cs和ss会有问题
	cs=SELECTOR(USER_CODE,PL_USER);
	ss=SELECTOR(USER_DATA,PL_USER);
	ds=SELECTOR(USER_DATA,PL_USER);
	es=SELECTOR(USER_DATA,PL_USER);
	fs=SELECTOR(USER_DATA,PL_USER);
	DEBUG("execve--ip:%x sp:%x\n",eip,esp);
	return 0;
}

	



