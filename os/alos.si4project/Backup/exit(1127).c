#include<linux/kernel.h>
#include<sys/types.h>
#include<linux/sched.h>
int sys_kill (int pid, int sig)
{
	panic("not support");

}
int sys_exit (int error_code)
{
	int i;
	for (i = 0; i < NR_TASKS; i++)
		if(task[i]==current)
			break;

	if(i==NR_TASKS)
		panic("current task not in task[i]");

	task[i]=NULL;
	//Ŀǰ�����ƣ�û���������ռ����Դ
	schedule();
	panic("can run here");
}

//// ϵͳ����waitpid()������ǰ���̣�ֱ��pid ָ�����ӽ����˳�����ֹ�������յ�Ҫ����ֹ
// �ý��̵��źţ���������Ҫ����һ���źž�����źŴ�����򣩡����pid ��ָ���ӽ�������
// �˳����ѳ���ν�Ľ������̣����򱾵��ý����̷��ء��ӽ���ʹ�õ�������Դ���ͷš�
// ���pid > 0, ��ʾ�ȴ����̺ŵ���pid ���ӽ��̡�
// ���pid = 0, ��ʾ�ȴ�������ŵ��ڵ�ǰ���̵��κ��ӽ��̡�
// ���pid < -1, ��ʾ�ȴ�������ŵ���pid ����ֵ���κ��ӽ��̡�
// [ ���pid = -1, ��ʾ�ȴ��κ��ӽ��̡�]
// ��options = WUNTRACED����ʾ����ӽ�����ֹͣ�ģ�Ҳ���Ϸ��ء�
// ��options = WNOHANG����ʾ���û���ӽ����˳�����ֹ�����Ϸ��ء�
// ���stat_addr ��Ϊ�գ���ͽ�״̬��Ϣ���浽���
int sys_waitpid (pid_t pid, unsigned long *stat_addr, int options)
{
	panic("not suppot");

}



