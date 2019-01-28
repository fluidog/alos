#include<linux/kernel.h>
#include<sys/types.h>
#include<linux/sched.h>

//// 向指定任务(*p)发送信号(sig)，权限为priv。
static inline int send_sig (long sig, task_struct *p, int priv)
{
	// 若信号不正确或任务指针为空则出错退出。
	if (!p || sig < 1 || sig > 32)
		return -EINVAL;
  
	// 有权 || 进程的有效用户相同 || 超级用户进程
	if (priv || (current->euid == p->euid) || suser ())
		p->signal |= (1 << (sig - 1));
	else
		return -EPERM;	
	return 0;
}

/* kill()系统调用可用于向任何进程或进程组发送任何信号。
	pid>0，发送给pid进程。
	pid=0，发送给同进程组的进程。
	pid=-1,发送给所有进程
	pid<-1，发送给进程组号=pid的进程。	
信号发送权限检查:
	超级用户或进程的euid相同，pid=0比较特殊，无需权限检查
注:如果信号sig 为0，则不发送信号，但仍会进行错误检查。如果成功则返回0。*/
int sys_kill (int pid, int sig)
{
	int err,i,retval = 0;

	if(!sig){	//检查信号
		for(i=0;i<NR_TASKS;i++)
			if(task[i]&&task[i]->pid == current->pid)
				retval=1;
		if(retval)	//如果存在此进程
			return 0;
		return -1;
	}

	if(!pid)	//相同进程组
		for(i=0;i<NR_TASKS;i++)
			if(task[i]&&task[i]->pgrp == current->pgrp)
				if(err=send_sig(sig,task[i],1))//始终有权眼
					retval=err;
	else if(pid>0)	//指定进程
		for(i=0;i<NR_TASKS;i++)
			if(task[i]&&task[i]->pid == current->pid)
				if(err=send_sig(sig,task[i],0))
					retval=err;
	else if(pid==-1)	//所有进程
		for(i=0;i<NR_TASKS;i++)
			if(task[i])
				if(err=send_sig(sig,task[i],0))
					retval=err;
	else 	//所有进程组号=pid的进程
		for(i=0;i<NR_TASKS;i++)
			if(task[i] &&task[i]->pgrp == -pid)
				if(err=send_sig(sig,task[i],0))
					retval=err;			
	return err;
}

//终止会话(session)。
static void kill_session (void)
{
	int i;
	for(i=0;i<NR_TASKS;i++)
		if(task[i]&&task[i]->session == current->session)
			task[i]->signal |= 1<<(SIGHUP - 1);	// 发送挂断进程信号
}

/*退出进程:
	1)释放用户空间
	2)过继子进程
	3)关闭相关结点
	4)如果是leader,则终止session
	5)如果是leader,且拥有tty，则释放tty
	6)设置当前进程状态和退出吗
	7)通知父进程，并调度*/
int sys_exit (int error_code)
{
	int i;
	free_page_table();//释放用户空间

	//过继子进程给进程0继续当儿子
	for (i = 0; i < NR_TASKS; i++){
		if (task[i] && task[i]->father == current->pid){
			task[i]->father = 0;		
		if (task[i]->state == TASK_ZOMBIE)
			send_sig (SIGCHLD, task[0], 1);
      }
	}

	//关闭当前进程打开着的所有文件。
  	for (i = 0; i < NR_OPEN; i++)
    	if (current->filp[i])
      		sys_close (i);

	//释放当前目录，根结点，执行文件结点
	iput (current->pwd);
	current->pwd = NULL;
	iput (current->root);
	current->root = NULL;
	iput (current->executable);
	current->executable = NULL;
	
	//如果当前进程是leader 进程，则终止所有相关进程。
	if (current->leader)
		kill_session ();
	//如果当前进程是领头(leader)进程并且其有控制的终端，则释放该终端。
	if (current->leader && current->tty >= 0)
		tty_table[current->tty].pgrp = 0;
	
	//把当前进程置为僵死状态，并设置退出码。
	current->state = TASK_ZOMBIE;
	current->exit_code = error_code;
	
	//通知父进程，也即向父进程发送信号SIGCHLD 。
	tell_father (current->father);
	schedule ();
}

//// 释放指定进程(任务)。
void release (task_struct *p)
{
	int i;
	if (!p)
		return;
	for (i = 1; i < NR_TASKS; i++){
		if (task[i] == p){
			task[i] = NULL;		// 置空该任务项并释放相关内存页。
			free_page ((u32) p);
			return;
		}
	}
  panic ("trying to release non-existent task");// 指定任务若不存在则死机。
}

/*系统调用waitpid()。挂起当前进程，直到pid 指定的子进程退出（终止）或者收到要求终止
	pid规则和kill相同
	options = WUNTRACED，表示如果子进程是停止的，也马上返回。
	options = WNOHANG，表示如果没有子进程退出或终止就马上返回。*/
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

		//找到符合pid条件的一个子进程，验证stat
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
	  put_fs_long (0x7f, stat_addr);	// 置状态信息为0x7f。
	  return (*p)->pid;	// 退出，返回子进程的进程号。
	case TASK_ZOMBIE:
	  current->cutime += (*p)->utime;	// 更新当前进程的子进程用户
	  current->cstime += (*p)->stime;	// 态和核心态运行时间。
	  flag = (*p)->pid;
	  code = (*p)->exit_code;	// 取子进程的退出码。
	  release (*p);		// 释放该子进程。
	  put_fs_long (code, stat_addr);	// 置状态信息为退出码值。
	  return flag;		// 退出，返回子进程的pid.
	default:
	  flag = 1;		// 如果子进程不在停止或僵死状态，则flag=1。
	  continue;
	}
  }
  if (flag)
  {				// 如果子进程没有处于退出或僵死状态，
	  if (options & WNOHANG)	// 并且options = WNOHANG，则立刻返回。
		return 0;
	  current->state = TASK_INTERRUPTIBLE;	// 置当前进程为可中断等待状态。
	  schedule ();		// 重新调度。
	  if (!(current->signal &= ~(1 << (SIGCHLD - 1))))	// 又开始执行本进程时，
		goto repeat;		// 如果进程没有收到除SIGCHLD 的信号，则还是重复处理。
	  else
		return -EINTR;		// 退出，返回出错码。
  }
  return -ECHILD;
}




