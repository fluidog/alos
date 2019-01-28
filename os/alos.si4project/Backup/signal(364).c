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

