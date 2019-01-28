#include <linux/sched.h>
#include <linux/kernel.h>
#include <memory.h>
#include <string.h>
#include <asm/system.h>
#include <asm/io.h>
#include <signal.h>

#define _S(nr) (1<<((nr)-1))
#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))

#define LATCH (1193180/HZ)

long volatile jiffies=0;	//系统从开机经过的滴答数
long startup_time;	//在cmos_init中初始化
task_struct *current;
task_struct * task[NR_TASKS];
long pid=0;		//用于新进程的pid，每次+1
struct tss_struct tss;	//所有进程共享同一个tss


// 显示任务号nr 的进程号、进程状态和内核堆栈空闲字节数（大约）。
void show_task (int nr)
{
	task_struct *p=task[nr];
	u32 *esp=(u32*)((u32)p+PAGE_SIZE);
	int i= 0, j = 4096 - sizeof (task_struct);

	printk ("%d: pid=%d, state=%d, ", nr, p->pid, p->state);
	while (i < j && !((char *) (p + 1))[i])	// 检测指定任务数据结构以后等于0 的字节数。
		i++;
	printk ("%d (of %d) chars free in kernel stack\n", i, j);

	
	DEBUG("ss:%x esp:%x eip:%x cs:%x eip:%x ds:%x es:%x fs:%x edx:%x ecx:%x ebx:%x "
			"eip:%x ebp:%x esi:%x edi:%x eax:%x\n",*(esp-1),*(esp-2),*(esp-3),*(esp-4),\
			*(esp-5),*(esp-6),*(esp-7),*(esp-8),*(esp-9),*(esp-10),\
			*(esp-11),*(esp-12),*(esp-13),*(esp-14),*(esp-15),*(esp-16));	
}

// 显示所有任务的任务号、进程号、进程状态和内核堆栈空闲字节数（大约）。
void show_stat (void)
{
	int i;

	for (i = 0; i < NR_TASKS; i++)// NR_TASKS 是系统能容纳的最大进程（任务）数量（64 个），
		if (task[i])		// 定义在include/kernel/sched.h 第4 行。
			show_task (i);
}


/*切换任务:
			0.保存所有状态
			1.current 指向新任务 
			2.tss->esp0 指向新任务内核堆栈
			3.切换页表(cr3)
			4.恢复esp,eip
			5.恢复所有状态*/
 void switch_to(int next)
 {
 	/* 保存现场 */
 	asm("pusha\n"
		"pushf\n"
		"mov %%esp,%0\n"
		"movl $1f,%1\n"
		:"=m"(current->pause_stack),"=m"(current->pause_eip):);


	/* 切换任务 */
	current=task[next];		//current
	current->tss->esp0=(u32)(current)+PAGE_SIZE; //tss
	asm("mov %0,%%cr3\n"	//cr3
		"mov %1,%%esp\n"	//esp
		"jmp *%2\n"			//eip
		::"g"(current->cr3),"m"(current->pause_stack),"m"(current->pause_eip));
		

	/* 恢复现场 */
	asm("1:popf\n"
		"popa\n");
	return ;
 }


//任务调度
void schedule(void)
{
	int i,next,c;
	task_struct ** p=task;	

	/* 检测alarm（进程的报警定时值），唤醒任何已得到信号的可中断任务 */
	for(i=0;i<NR_TASKS;i++){
		//闹钟
		if(task[i] && task[i]->alarm && task[i]->alarm < jiffies){
			task[i]->signal |= (1 << (SIGALRM - 1));
			task[i]->alarm = 0;
			DEBUG("alarm: task%d\n",i);
		}
		//如果有非屏蔽信号，则唤醒任务
		//目前不唤醒，因为唤醒后，应该longjmp直接执行信号捕捉函数，而非原路返回，但是，目前还未实现
		/*
		if( task[i] && (task[i]->signal &~task[i]->blocked) && 
				task[i]->state==TASK_INTERRUPTIBLE ){
			task[i]->state = TASK_RUNNING;	//置为就绪（可执行）状态
			DEBUG("signal: wakeup interruptible task%d\n",i);
		}*/
	}

	//找出最应该调度的任务
	while(1){
		c=0;
		next=0;
		for(i=0;i<NR_TASKS;i++){
			if(task[i] && task[i]->state==TASK_RUNNING && task[i]->counter>c)
				c=task[i]->counter,next=i;
		}
		if(c)
			break;
		for(i=0;i<NR_TASKS;i++){
			if(task[i])
				task[i]->counter=task[i]->priority;
		}
	}
	if(current!=task[next]){
		//DEBUG("switch to%d\n",next);
		switch_to(next);
	}
}

//睡眠直到被信号唤醒
int sys_pause(void)
{
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	return 0;
}

//镶嵌唤醒当前任务之前睡眠的进程
void sleep_on(task_struct **p)
{
	task_struct *tmp;
	
	if(!p)
		return ;
	//DEBUG("sleep_on:%d\n",current->pid);
	
	tmp = *p;
	*p = current;

	current->state = TASK_UNINTERRUPTIBLE;
	schedule();

	if(tmp)
		tmp->state = 0;
}


/* 永远唤醒最新被睡眠的进程 （即使唤醒过程中，有新进程被睡了）*/
void interruptible_sleep_on(task_struct **p)
{
	task_struct *tmp;
	if(!p)
		return ;
	//DEBUG("interruptible_sleep_on:%d\n",current->pid);

	tmp = *p;
	*p = current;
	
	while(1){		/* (*p && *p!=current) 时继续睡眠 */
		current->state = TASK_INTERRUPTIBLE;
		schedule();

		if(*p==NULL || *p==current)
			break;
		(**p).state = 0;
	}
	*p=NULL;//*p=tmp
	if(tmp)
		tmp->state = 0;
	
}

void wake_up(task_struct **p)
{
	if (p && *p) {
		(**p).state=0;
		*p=NULL;
	}
}


void do_timer(int cpl)
{
	if(cpl){
		current->utime++;
		current->counter--;
	}
	else
		current->stime++;
	
	
	if(!current->counter)
		if(cpl)
			schedule();
		else
			current->counter++;
}

//设置闹钟，返回之前闹钟剩余时间
int sys_alarm(long seconds)
{
	int old = current->alarm;

	if (old)
		old = (old - jiffies) / HZ;
	current->alarm = (seconds>0)?(jiffies+HZ*seconds):0;
	return (old);
}

int sys_getpid(void)
{
	return current->pid;
}

int sys_getppid(void)
{
	return current->father;
}

int sys_getuid(void)
{
	return current->uid;
}

int sys_geteuid(void)
{
	return current->euid;
}

int sys_getgid(void)
{
	return current->gid;
}

int sys_getegid(void)
{
	return current->egid;
}

// 系统调用功能 -- 降低对CPU 的使用优先权（有人会用吗？?）。
// 应该限制increment 大于0，否则的话,可使优先权增大！！
int sys_nice (long increment)
{
	if (current->priority - increment > 0)
		current->priority -= increment;
	return 0;
}


void sched_init(void)
{	
	jiffies=0;//从开机到现在经过的滴答数，虽然只有时钟中断开启后才有效，但放在前面初始化总没错
	pid=0;	
	
	memset(task,0,sizeof(task));	
	current=task[0]=(task_struct *)(get_free_page()+VM_START);	
	memset(current,0,PAGE_SIZE);

	//有很多都是默认的0，可以不用再次赋0，但是为了体现进程所有的数据，所以也写出来了
	current->state=TASK_RUNNING;
	current->priority=20;
	current->counter=20;
	current->signal=0;	//无信号
	//sigaction[i]=0; //默认
	current->blocked=0;	//无阻塞
	
	
	//exit_code,start_code,end_code,end_data,brk,start_stack,mem_szie在execve此进程时设置
	current->pgrp=current->father=current->pid=pid++;				//进程号
	current->leader=current->session=0;
	current->suid=current->euid=current->uid=0;		//用户号
	current->gid=current->egid=current->sgid=current->uid;	//用户组号
	
	current->alarm=0;	//无闹钟
	current->cstime=current->cutime=current->utime=current->stime=0;
	current->start_time=jiffies;
	
	current->tty=-1;	//默认无tty终端
	current->umask=0;
	//pwd,root,executable在挂载根文件或execve此进程时设置
	current->close_on_exec=0;	//默认不关闭
	//filp[NR_OPEN]=0;	//默认
	
	//所有进程共享同一个TSS	
	current->tss=&tss;
	current->tss->esp0=(unsigned long)current+PAGE_SIZE;
	current->tss->ss0=SELECTOR(KERNEL_DATA,PL_KERNEL);
	asm("mov %%cr3,%%eax\n"	//当前页表设为task[0]的页表
		"mov %%eax,%0"
		:"=m"(current->cr3)::"eax");
	
	/*进程0的起始堆栈，目前是VM_START,应该设为current+PAGE_SIZE，
	但是，这样便要复制堆栈里的内容，否则，无法返回，于是设置堆栈就放在main()函数里，
	这样，就可以直接丢弃堆栈内容*/
	//esp=(unsigned long)current+PAGE_SIZE;	
}
