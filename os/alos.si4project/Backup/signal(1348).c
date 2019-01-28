#include <linux/sched.h>	
#include <linux/kernel.h>	// 内核头文件。含有一些内核常用函数的原形定义。
#include <signal.h>		// 信号头文件。定义信号符号常量，信号结构以及信号操作函数原型。
#include <asm/segment.h>	//段操作

extern int sys_exit (int error_code);

// 获取当前任务信号屏蔽位图（屏蔽码）。
int sys_sgetmask ()
{
	return current->blocked;
}

// 设置新的信号屏蔽位图。SIGKILL 不能被屏蔽。返回值是原信号屏蔽位图。
int sys_ssetmask (int newmask)
{
	int old = current->blocked;
	current->blocked = newmask & ~(1 << (SIGKILL - 1));
	return old;
}

// 复制sigaction 数据到fs 数据段to 处，from指向内核数据，to为用户缓冲区
static inline void save_old (char *from,char *to)
{
	int i=sizeof (struct sigaction);
	verify_area (to, i);	// 验证to 处的内存是否足够。
	
	while(i--)
		put_fs_byte (*(from++), to++);
}

//从用户缓冲区复制sigaction数据到内核
static inline void get_new (char *from, char *to)
{
	int i=sizeof (struct sigaction);
	while(i--)
		*(to++) = get_fs_byte (from++);
}


// 安装信号捕捉函数signal()系统调用,尽量用sigaction()
int sys_signal (int signum, long handler, long restorer)
{
	struct sigaction tmp;
	if (signum < 1 || signum > 32 || signum == SIGKILL)	//信号有效
		return -1;
	
	//构造新action
	tmp.sa_handler = (void (*)(int)) handler;
	//句柄只能使用一次。信号处理中，不重新设置mask，也即不能屏蔽自己或指定信号
	tmp.sa_flags = SA_ONESHOT | SA_NOMASK;
	tmp.sa_restorer = (void (*)(void)) restorer;//返回地址

	//保存原action，并设置新action
	handler = (long) current->sigaction[signum - 1].sa_handler;
	current->sigaction[signum - 1] = tmp;
	return handler;
}

// sigaction()系统调用，如果lodaction!=NULL,则保存原句柄
//注意:没有根据flag的NOMASK来设置sigaction的mask，因为，我认为这不符合常理
int sys_sigaction (int signum, const struct sigaction *action,
					struct sigaction *oldaction)
{
	struct sigaction tmp;
	if (signum < 1 || signum > 32 || signum == SIGKILL)	//信号有效
		return -1;

	//保存原action
	if (oldaction)
		save_old( (char *)(signum-1+current->sigaction),(char *)oldaction );
	//设置新action
	get_new ( (char *) action,(char *)(signum-1+current->sigaction) );
	
	return 0;
}
					
//信号处理函数，在返回内核返回用户态时触发才有效
void do_signal (long eax, long ebx, long ecx, long edx,
			long fs, long es, long ds,long eip, long cs, long eflags, unsigned long *esp, long ss)
{
	unsigned long sa_handler;
	long signr=0,sign;
	long old_eip = eip;
	struct sigaction *sa; 
	int longs;
	unsigned long *tmp_esp;
	//如果不是将要返回用户模式，也即内核模式时，发生中断的返回，不处理信号
	if(!(cs&0x3))
		return ;
	//无非屏蔽信号，则返回(应该添加非返回用户模式，则返回)
	sign = current->signal & ~current->blocked;
	if(!sign)
		return ;
	//查找信号位图最前面的信号，并转换为序列号,这段可以用bsf和btr嵌入汇编优化，但这样看起来简单
	signr = 0;
	while(!(sign&0x1)){
		signr++;
		sign >>= 1;
	}
	current->signal &= ~(1<<signr);//复位处理信号

	/*				处理信号							*/
	sa=&current->sigaction[signr];
	sa_handler=(unsigned long)sa->sa_handler;
	if (sa_handler == 1)//忽视
		return;
	if (!sa_handler)//默认
	{
		if (signr == SIGCHLD)
			return;
		else
			sys_exit (1 << (signr));//终止
	}
	// 如果该信号句柄只需使用一次，则将该句柄置空(该信号句柄已经保存在sa_handler 指针中)。
	if (sa->sa_flags & SA_ONESHOT)
		sa->sa_handler = NULL;
	
	/*			填充用户堆栈和内核堆栈						*/
	eip = sa_handler;//返回地址改为信号处理函数
		
	esp -= 9;
	tmp_esp = esp;
	verify_area (esp, 9 * 4);	
// 在用户堆栈中从下到上存放sa_restorer, 信号signr, 屏蔽码blocked
// eax, ecx, edx, eflags 和用户程序原代码指针。
	put_fs_long ((u32) sa->sa_restorer, tmp_esp++);	//sa_resorer
	put_fs_long (signr, tmp_esp++);			//signr
	put_fs_long (current->blocked, tmp_esp++);	//保留原mask
	put_fs_long (eax, tmp_esp++);				//eax
	put_fs_long (ebx,tmp_esp++);				//源程序不知道为什么不保存ebx
	put_fs_long (ecx, tmp_esp++);				//ecx
	put_fs_long (edx, tmp_esp++);				//edx
	put_fs_long (eflags, tmp_esp++);			//eflags
	put_fs_long (old_eip, tmp_esp++);			//old_eip
	
	//如果没有设置NOMASK,则表示sa_mask有效
	if (!(sa->sa_flags & SA_NOMASK))
		current->blocked |= sa->sa_mask;	// 进程阻塞码(屏蔽码)添上sa_mask 中的码位。
}


int sys_sigreturn (long ebx, long ecx, long edx,
			long fs, long es, long ds,long eip, long cs, long eflags, unsigned long *esp, long ss)
{
	//恢复为信号发生前的状态
	
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

