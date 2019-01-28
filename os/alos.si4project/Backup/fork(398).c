#include<linux/sched.h>
#include<string.h>
#include<memory.h>
#include<linux/kernel.h>

int sys_fork();
asm(".text\n.global sys_fork\n"
		"sys_fork:"
		"push %ebp\n"	//保存状态
		"push %esi\n"
		"push %edi\n"
		"call copy_process\n"//复制进程数据结构
		"push %eax\n"
		"push %esp\n"
		"call set_ret_stat\n"//设置子进程断点esp和eip，并设置子进程返回值
		"add $4,%esp\n"		//子进程从此进入
		"pop %eax\n"		//返回值
		"pop %edi\n"
		"pop %esi\n"
		"pop %ebp\n"
		"ret");

static int copy_process()
{	
	int i,ret,tmp;
	task_struct *newTask;
	for(i=0;i<NR_TASKS;i++){
		if(!task[i])
			break;
	}
	if(i==NR_TASKS)
		return -1;
	
	if(!(tmp=get_free_page()))
		return -1;
	newTask=(task_struct *)(tmp+VM_START);
	task[i]=newTask;
	ret=i;	
	
	memcpy(newTask,current,PAGE_SIZE);//千万把这个放在上面，否则下面的赋值都无效
	
	//会使newTask->cr3指向新的页目录表，得的新的用户地址空间，共享内核空间
	if(copy_page_table(newTask)<0)	
		return -1;

	//进程标识、状态
	newTask->pid=pid++;
	newTask->father=current->pid;
	newTask->leader=0;//不是leader
	newTask->utime=newTask->stime=newTask->cutime=newTask->cstime=0;
	newTask->start_time=jiffies;

	//相关结点+1
	newTask->pwd->i_count++;
	newTask->root->i_count++;
	newTask->executable->i_count++;
	//根据close_on_exec标志，复制打开文件
	for(i=0;i<NR_OPEN;i++){
		if( current->filp[i] ){
			if( (current->close_on_exec>>i)&0x1 ){ 	//关闭
				newTask->filp[i]=NULL;
			}
			else{	//复制
				newTask->filp[i]->f_count++;
			}		
		}		
	}
	return ret;
}

//设置子进程断点状态(pause_eip,pause_stack)
static void set_ret_stat(u32 esp,int next){
	if(next<0)
		return ;

	//子进程断点ip
	asm("mov %1,%0"
		:"=g"(task[next]->pause_eip):"m"(*(&esp-1)));
	//子进程内核堆栈
	esp=(u32)task[next]+esp%PAGE_SIZE-4;
	asm("mov %1,%0"
		:"=m"(task[next]->pause_stack):"g"(esp));
	//子进程返回值0
	asm("movl $0,%0"
		:"=m"(*((u32 *)esp+1)):);

	//父进程返回子进程pid
	next=task[next]->pid;
	schedule();//让子进程先运行
}  


