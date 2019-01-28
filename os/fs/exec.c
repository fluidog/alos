/*Ŀǰexecve���ܽ���elf�����ļ������filename�ļ�������bin��ʽ����,
	���ң�start_addr�ٶ�Ϊ0x0��end_dataΪ�ļ���С,
	start_stackΪ0xc0000000(VM_START),
	end_code��brk��ʱ��֧��
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
	if( !permission(execfile, 1) )//���ִ��Ȩ��
		return -1;
	if(!S_ISREG(execfile->i_mode))//���ǿ�ִ���ļ�
		return -1;

	if(current->executable)//��һ�����̸տ�ʼû��executable
		iput(current->executable);
	current->executable=execfile;
	
	//֮�����ӽ���elf��ʽ֮�󣬸���elf������
	current->start_code=0x2000;
	current->end_data=execfile->i_size+current->start_code;
	current->start_stack=0x6000;

	//�����û��ռ�
	free_page_table();
	
	eip=current->start_code;
	esp=current->start_stack;
	//cs,ss,ds,es,fs�����Բ�����,���ǿ��ǵ���һ�������Ǵ��ں˽���˺����ģ�cs��ss��������
	cs=SELECTOR(USER_CODE,PL_USER);
	ss=SELECTOR(USER_DATA,PL_USER);
	ds=SELECTOR(USER_DATA,PL_USER);
	es=SELECTOR(USER_DATA,PL_USER);
	fs=SELECTOR(USER_DATA,PL_USER);
	DEBUG("execve--ip:%x sp:%x\n",eip,esp);
	return 0;
}

	



