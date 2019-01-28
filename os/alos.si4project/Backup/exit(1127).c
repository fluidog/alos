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
	//目前不完善，没有清除任务占用资源
	schedule();
	panic("can run here");
}

//// 系统调用waitpid()。挂起当前进程，直到pid 指定的子进程退出（终止）或者收到要求终止
// 该进程的信号，或者是需要调用一个信号句柄（信号处理程序）。如果pid 所指的子进程早已
// 退出（已成所谓的僵死进程），则本调用将立刻返回。子进程使用的所有资源将释放。
// 如果pid > 0, 表示等待进程号等于pid 的子进程。
// 如果pid = 0, 表示等待进程组号等于当前进程的任何子进程。
// 如果pid < -1, 表示等待进程组号等于pid 绝对值的任何子进程。
// [ 如果pid = -1, 表示等待任何子进程。]
// 若options = WUNTRACED，表示如果子进程是停止的，也马上返回。
// 若options = WNOHANG，表示如果没有子进程退出或终止就马上返回。
// 如果stat_addr 不为空，则就将状态信息保存到那里。
int sys_waitpid (pid_t pid, unsigned long *stat_addr, int options)
{
	panic("not suppot");

}



