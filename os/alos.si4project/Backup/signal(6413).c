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
	panic("not support");
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
void do_signal (long signr, long eax, long ebx, long ecx, long edx,
			long fs, long es, long ds,long eip, long cs, long eflags, unsigned long *esp, long ss)
{
	unsigned long sa_handler;
	long old_eip = eip;
	struct sigaction *sa = current->sigaction + signr - 1;	//current->sigaction[signu-1]��
	int longs;
	unsigned long *tmp_esp;

	/*ret_code��ʾ���µĶ����ƴ���
	mov __NR_sigreturn,al; int 0x80;*/
	u8 ret_code[4]={0xb0,__NR_sigreturn,0xcd,0x80};

	sa_handler = (unsigned long) sa->sa_handler;
// ����źž��ΪSIG_IGN(����)���򷵻أ�������ΪSIG_DFL(Ĭ�ϴ���)��������ź���
// SIGCHLD �򷵻أ�������ֹ���̵�ִ��
	if (sa_handler == 1)
		return;
	if (!sa_handler)
	{
		if (signr == SIGCHLD)
			return;
		else
// ����Ӧ����do_exit(1<<signr))��
			do_exit (1 << (signr - 1));	// [?? Ϊʲô���ź�λͼΪ��������Ϊʲô!��?]
	}
// ������źž��ֻ��ʹ��һ�Σ��򽫸þ���ÿ�(���źž���Ѿ�������sa_handler ָ����)��
	if (sa->sa_flags & SA_ONESHOT)
		sa->sa_handler = NULL;
// ������δ��뽫�źŴ��������뵽�û���ջ�У�ͬʱҲ��sa_restorer,signr,����������(���
// SA_NOMASK û��λ),eax,ecx,edx ��Ϊ�����Լ�ԭ����ϵͳ���õĳ��򷵻�ָ�뼰��־�Ĵ���ֵ
// ѹ���ջ������ڱ���ϵͳ�����ж�(0x80)�����û�����ʱ������ִ���û����źž������Ȼ��
// �ټ���ִ���û�����
// ���û�����ϵͳ���õĴ���ָ��eip ָ����źŴ�������
	*(&eip) = sa_handler;
// ��������ź��Լ��Ĵ������յ��ź��Լ�����Ҳ��Ҫ�����̵�������ѹ���ջ��
	longs = (sa->sa_flags & SA_NOMASK) ? 7 : 8;
// ��ԭ���ó�����û��Ķ�ջָ��������չ7����8�������֣�������ŵ����źž���Ĳ����ȣ���
// ������ڴ�ʹ���������������ڴ泬���������ҳ�ȣ���
	*(&esp) -= longs;
	verify_area (esp, longs * 4);
// ���û���ջ�д��µ��ϴ��sa_restorer, �ź�signr, ������blocked(���SA_NOMASK ��λ),
// eax, ecx, edx, eflags ���û�����ԭ����ָ�롣
	tmp_esp = esp;
	put_fs_long ((long) sa->sa_restorer, tmp_esp++);
	put_fs_long (signr, tmp_esp++);
	if (!(sa->sa_flags & SA_NOMASK))
		put_fs_long (current->blocked, tmp_esp++);
	put_fs_long (eax, tmp_esp++);
	put_fs_long (ecx, tmp_esp++);
	put_fs_long (edx, tmp_esp++);
	put_fs_long (eflags, tmp_esp++);
	put_fs_long (old_eip, tmp_esp++);
	current->blocked |= sa->sa_mask;	// ����������(������)����sa_mask �е���λ��
}



