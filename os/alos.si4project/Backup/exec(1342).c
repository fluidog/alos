/*Ŀǰexecve���ܽ���elf�����ļ������filename�ļ�������bin��ʽ����,
	���ң�start_addr�ٶ�Ϊ0x0��end_dataΪ�ļ���С,
	start_stackΪ0xc0000000(VM_START),
	end_code��brk��ʱ��֧��
*/
#include<linux/fs.h>
#include<memory.h>
#include<sys/stat.h>
extern int permission(m_inode * inode,int mask);



int sys_execve(const char * filename, char ** argv, char ** envp)
{
	m_inode *execfile;	
	DEBUG("exec:%s\n",filename);
	if( !(execfile=namei(filename)) )
		return -1;

	if( !permission(execfile, 1) )//���ִ��Ȩ��
		return -1;
	
	if(!S_ISREG(execfile->i_mode))//���ǿ�ִ���ļ�
		return -1;
		
	current->executable=execfile;
	current->start_code=0x2000;
	current->end_data=execfile->i_size+current->start_code;
	current->start_stack=0x6000;
	free_page_table();
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

	



