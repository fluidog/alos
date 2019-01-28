#include<linux/kernel.h>
#include<sys/types.h>
#include<linux/sched.h>

//// ��ָ������(*p)�����ź�(sig)��Ȩ��Ϊpriv��
static inline int send_sig (long sig, task_struct *p, int priv)
{
	// ���źŲ���ȷ������ָ��Ϊ��������˳���
	if (!p || sig < 1 || sig > 32)
		return -EINVAL;
  
	// ��Ȩ || ���̵���Ч�û���ͬ || �����û�����
	if (priv || (current->euid == p->euid) || suser ())
		p->signal |= (1 << (sig - 1));
	else
		return -EPERM;	
	return 0;
}

/* kill()ϵͳ���ÿ��������κν��̻�����鷢���κ��źš�
	pid>0�����͸�pid���̡�
	pid=0�����͸�ͬ������Ľ��̡�
	pid=-1,���͸����н���
	pid<-1�����͸��������=pid�Ľ��̡�	
�źŷ���Ȩ�޼��:
	�����û�����̵�euid��ͬ��pid=0�Ƚ����⣬����Ȩ�޼��
ע:����ź�sig Ϊ0���򲻷����źţ����Ի���д����顣����ɹ��򷵻�0��*/
int sys_kill (int pid, int sig)
{
	int err,i,retval = 0;

	if(!sig){	//����ź�
		for(i=0;i<NR_TASKS;i++)
			if(task[i]&&task[i]->pid == current->pid)
				retval=1;
		if(retval)	//������ڴ˽���
			return 0;
		return -1;
	}

	if(!pid)	//��ͬ������
		for(i=0;i<NR_TASKS;i++)
			if(task[i]&&task[i]->pgrp == current->pgrp)
				if(err=send_sig(sig,task[i],1))//ʼ����Ȩ��
					retval=err;
	else if(pid>0)	//ָ������
		for(i=0;i<NR_TASKS;i++)
			if(task[i]&&task[i]->pid == current->pid)
				if(err=send_sig(sig,task[i],0))
					retval=err;
	else if(pid==-1)	//���н���
		for(i=0;i<NR_TASKS;i++)
			if(task[i])
				if(err=send_sig(sig,task[i],0))
					retval=err;
	else 	//���н������=pid�Ľ���
		for(i=0;i<NR_TASKS;i++)
			if(task[i] &&task[i]->pgrp == -pid)
				if(err=send_sig(sig,task[i],0))
					retval=err;			
	return err;
}

//��ֹ�Ự(session)��
static void kill_session (void)
{
	int i;
	for(i=0;i<NR_TASKS;i++)
		if(task[i]&&task[i]->session == current->session)
			task[i]->signal |= 1<<(SIGHUP - 1);	// ���͹ҶϽ����ź�
}

/*�˳�����:
	1)�ͷ��û��ռ�
	2)�����ӽ���
	3)�ر���ؽ��
	4)�����leader,����ֹsession
	5)�����leader,��ӵ��tty�����ͷ�tty
	6)���õ�ǰ����״̬���˳���
	7)֪ͨ�����̣�������*/
int sys_exit (int error_code)
{
	int i;
	free_page_table();//�ͷ��û��ռ�

	//�����ӽ��̸�����0����������
	for (i = 0; i < NR_TASKS; i++){
		if (task[i] && task[i]->father == current->pid){
			task[i]->father = 0;		
		if (task[i]->state == TASK_ZOMBIE)
			send_sig (SIGCHLD, task[0], 1);
      }
	}

	//�رյ�ǰ���̴��ŵ������ļ���
  	for (i = 0; i < NR_OPEN; i++)
    	if (current->filp[i])
      		sys_close (i);

	//�ͷŵ�ǰĿ¼������㣬ִ���ļ����
	iput (current->pwd);
	current->pwd = NULL;
	iput (current->root);
	current->root = NULL;
	iput (current->executable);
	current->executable = NULL;
	
	//�����ǰ������leader ���̣�����ֹ������ؽ��̡�
	if (current->leader)
		kill_session ();
	//�����ǰ��������ͷ(leader)���̲������п��Ƶ��նˣ����ͷŸ��նˡ�
	if (current->leader && current->tty >= 0)
		tty_table[current->tty].pgrp = 0;
	
	//�ѵ�ǰ������Ϊ����״̬���������˳��롣
	current->state = TASK_ZOMBIE;
	current->exit_code = error_code;
	
	//֪ͨ�����̣�Ҳ���򸸽��̷����ź�SIGCHLD ��
	tell_father (current->father);
	schedule ();
}

//// �ͷ�ָ������(����)��
void release (task_struct *p)
{
	int i;
	if (!p)
		return;
	for (i = 1; i < NR_TASKS; i++){
		if (task[i] == p){
			task[i] = NULL;		// �ÿո�������ͷ�����ڴ�ҳ��
			free_page ((u32) p);
			return;
		}
	}
  panic ("trying to release non-existent task");// ָ����������������������
}

/*ϵͳ����waitpid()������ǰ���̣�ֱ��pid ָ�����ӽ����˳�����ֹ�������յ�Ҫ����ֹ
	pid�����kill��ͬ
	options = WUNTRACED����ʾ����ӽ�����ֹͣ�ģ�Ҳ���Ϸ��ء�
	options = WNOHANG����ʾ���û���ӽ����˳�����ֹ�����Ϸ��ء�*/
int sys_waitpid (pid_t pid, unsigned long *stat, int options)
{
	int flag, code,i;

	verify_area (stat, sizeof(*stat));
	
repeat:
	flag = 0;
	for(i=0;i<NR_TASKS;i++){
		if(!task[i] || task[i]==current)
			continue;
		if(task[i]->father != current->pid)
			continue;

		if(pid>0)
			if(task[i]->pid != pid)
				continue;
		if(!pid)
			if(task[i]->pgrp != current->pgrp)
				continue;
		if(pid!=-1)
			if(task[i]->pgrp != -pid)
				continue;

		//�ҵ�����pid������һ���ӽ��̣���֤stat
		switch(task[i]->state){
		case TASK_STOPPED:
			if(!(options & WUNTRACED))
				continue;
			put_fs_long(0x7f,stat);
			return task[i]->pid;
		case TASK_ZOMBIE:
			current->cutime += task[i]->utime;
			current->cstime += task[i]->stime;
			flag=task[i]->pid;
			code=task[i]->exit_code;
			release(task[i]);
			put_fs_long(code,stat);
			return flag;
		}


    switch ((*p)->state)
	{
	case TASK_STOPPED:
	  if (!(options & WUNTRACED))
	    continue;
	  put_fs_long (0x7f, stat_addr);	// ��״̬��ϢΪ0x7f��
	  return (*p)->pid;	// �˳��������ӽ��̵Ľ��̺š�
	case TASK_ZOMBIE:
	  current->cutime += (*p)->utime;	// ���µ�ǰ���̵��ӽ����û�
	  current->cstime += (*p)->stime;	// ̬�ͺ���̬����ʱ�䡣
	  flag = (*p)->pid;
	  code = (*p)->exit_code;	// ȡ�ӽ��̵��˳��롣
	  release (*p);		// �ͷŸ��ӽ��̡�
	  put_fs_long (code, stat_addr);	// ��״̬��ϢΪ�˳���ֵ��
	  return flag;		// �˳��������ӽ��̵�pid.
	default:
	  flag = 1;		// ����ӽ��̲���ֹͣ����״̬����flag=1��
	  continue;
	}
  }
  if (flag)
  {				// ����ӽ���û�д����˳�����״̬��
	  if (options & WNOHANG)	// ����options = WNOHANG�������̷��ء�
		return 0;
	  current->state = TASK_INTERRUPTIBLE;	// �õ�ǰ����Ϊ���жϵȴ�״̬��
	  schedule ();		// ���µ��ȡ�
	  if (!(current->signal &= ~(1 << (SIGCHLD - 1))))	// �ֿ�ʼִ�б�����ʱ��
		goto repeat;		// �������û���յ���SIGCHLD ���źţ������ظ�����
	  else
		return -EINTR;		// �˳������س����롣
  }
  return -ECHILD;
}




