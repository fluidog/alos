/* 
* linux/kernel/signal.c
*
* (C) 1991 Linus Torvalds
*/
/*
ע�⣺signal.c��fork.c�ļ��ı���ѡ���ڲ�����vc�����Ż�ѡ��/Og����Ϊ�������ļ�
	�ڵĺ��������ڰ����˺������ص�ַ�����ݡ��������/Ogѡ��������ͻ�����Ϊ
	��Щ��������ʹ�ú�ռ�ø��ڴ棬���º�������ʱ����
	math/math_emulate.c����ҲӦ������������������û�а�eip�Ȳ����Ż���:)
*/

#include <linux/sched.h>	// ���ȳ���ͷ�ļ�������������ṹtask_struct����ʼ����0 �����ݣ�
// ����һЩ�й��������������úͻ�ȡ��Ƕ��ʽ��ຯ������䡣
#include <linux/kernel.h>	// �ں�ͷ�ļ�������һЩ�ں˳��ú�����ԭ�ζ��塣
#include <asm/segment.h>	// �β���ͷ�ļ����������йضμĴ���������Ƕ��ʽ��ຯ����

#include <signal.h>		// �ź�ͷ�ļ��������źŷ��ų������źŽṹ�Լ��źŲ�������ԭ�͡�

#include<linux/sys.h>
// ��ȡ��ǰ�����ź�����λͼ�������룩��
int sys_sgetmask ()
{
	return current->blocked;
}

// �����µ��ź�����λͼ��SIGKILL ���ܱ����Ρ�����ֵ��ԭ�ź�����λͼ��
int sys_ssetmask (int newmask)
{
	panic("not support");
}



// signal()ϵͳ���á�������sigaction()��Ϊָ�����źŰ�װ�µ��źž��(�źŴ������)��
// �źž���������û�ָ���ĺ�����Ҳ������SIG_DFL��Ĭ�Ͼ������SIG_IGN�����ԣ���
// ����signum --ָ�����źţ�handler -- ָ���ľ����restorer �Cԭ����ǰִ�еĵ�ַλ�á�
// ��������ԭ�źž����
int sys_signal (int signum, long handler, long restorer)
{
	struct sigaction tmp;

	if (signum < 1 || signum > 32 || signum == SIGKILL)	// �ź�ֵҪ�ڣ�1-32����Χ�ڣ�
		return -1;			// ���Ҳ�����SIGKILL��
	tmp.sa_handler = (void (*)(int)) handler;	// ָ�����źŴ�������
	tmp.sa_mask = 0;		// ִ��ʱ���ź������롣
	tmp.sa_flags = SA_ONESHOT | SA_NOMASK;	// �þ��ֻʹ��1 �κ�ͻָ���Ĭ��ֵ��
// �������ź����Լ��Ĵ��������յ���
	tmp.sa_restorer = (void (*)(void)) restorer;	// ���淵�ص�ַ��
	handler = (long) current->sigaction[signum - 1].sa_handler;
	current->sigaction[signum - 1] = tmp;
	return handler;
}
// sigaction()ϵͳ���á��ı�������յ�һ���ź�ʱ�Ĳ�����signum �ǳ���SIGKILL ������κ�
// �źš�[����²���(action)��Ϊ��]���²�������װ�����oldaction ָ�벻Ϊ�գ���ԭ����
// ��������oldaction���ɹ��򷵻�0������Ϊ-1��
int sys_sigaction (int signum, const struct sigaction *action,
					struct sigaction *oldaction)
{
	panic("not support");
	return 0;
}
/*
 ϵͳ�����жϴ���������������źŴ��������kernel/system_call.s,119 �У���
 �öδ������Ҫ�����ǽ��źŵĴ��������뵽�û������ջ�У����ڱ�ϵͳ���ý���
 ���غ�����ִ���źž������Ȼ�����ִ���û��ĳ���*/
void do_signal (long eax, long ebx, long ecx, long edx,
			long fs, long es, long ds,long eip, long cs, long eflags, unsigned long *esp, long ss)
{
	unsigned long sa_handler;
	long signr=0,sign;
	long old_eip = eip;
	struct sigaction *sa; 
	int longs;
	unsigned long *tmp_esp;
	//������ǽ�Ҫ�����û�ģʽ��Ҳ���ں�ģʽʱ�������жϵķ��أ��������ź�
	if(!(cs&0x3))
		return ;
	//�޷������źţ��򷵻�(Ӧ����ӷǷ����û�ģʽ���򷵻�)
	sign = current->signal & ~current->blocked;
	if(!sign)
		return ;
	//�����ź�λͼ��ǰ����źţ���ת��Ϊ���к�,��ο�����bsf��btrǶ�����Ż�����������������
	signr = 0;
	while(!(sign&0x1)){
		signr++;
		sign >>= 1;
	}
	current->signal &= ~(1<<signr);//��λ�����ź�

	/*				�����ź�							*/
	sa=&current->sigaction[signr];
	sa_handler=(unsigned long)sa->sa_handler;
	if (sa_handler == 1)//����
		return;
	if (!sa_handler)//Ĭ��
	{
		if (signr == SIGCHLD)
			return;
		else
			sys_exit (1 << (signr));//��ֹ
	}
	// ������źž��ֻ��ʹ��һ�Σ��򽫸þ���ÿ�(���źž���Ѿ�������sa_handler ָ����)��
	if (sa->sa_flags & SA_ONESHOT)
		sa->sa_handler = NULL;
	
	/*			����û���ջ���ں˶�ջ						*/
	eip = sa_handler;//���ص�ַ��Ϊ�źŴ�����
		
	esp -= 9;
	tmp_esp = esp;
	verify_area (esp, 9 * 4);	
// ���û���ջ�д��µ��ϴ��sa_restorer, �ź�signr, ������blocked
// eax, ecx, edx, eflags ���û�����ԭ����ָ�롣
	put_fs_long ((u32) sa->sa_restorer, tmp_esp++);	//sa_resorer
	put_fs_long (signr, tmp_esp++);			//signr
	put_fs_long (current->blocked, tmp_esp++);	//����ԭmask
	put_fs_long (eax, tmp_esp++);				//eax
	put_fs_long (ebx,tmp_esp++);				//Դ����֪��Ϊʲô������ebx
	put_fs_long (ecx, tmp_esp++);				//ecx
	put_fs_long (edx, tmp_esp++);				//edx
	put_fs_long (eflags, tmp_esp++);			//eflags
	put_fs_long (old_eip, tmp_esp++);			//old_eip
	
	//���û������NOMASK,���ʾsa_mask��Ч
	if (!(sa->sa_flags & SA_NOMASK))
		current->blocked |= sa->sa_mask;	// ����������(������)����sa_mask �е���λ��
}


int sys_sigreturn (long ebx, long ecx, long edx,
			long fs, long es, long ds,long eip, long cs, long eflags, unsigned long *esp, long ss)
{
	//�ָ�Ϊ�źŷ���ǰ��״̬
	
	long signr,eax;
	struct sigaction *sa;
	signr=get_fs_long(esp++);
	sa=&current->sigaction[signr];
	
	current->blocked=get_fs_long(esp++);
	eax=get_fs_long(esp++);
	ebx=get_fs_long(esp++);
	ecx=get_fs_long(esp++);
	edx=get_fs_long(esp++);
	eflags=get_fs_long(esp++);
	eip=get_fs_long(esp++);
	return eax;
}

