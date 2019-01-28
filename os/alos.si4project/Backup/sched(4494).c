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

long volatile jiffies=0;
long startup_time=0;
task_struct *current;
task_struct * task[NR_TASKS];

struct tss_struct tss;	//���н��̹���ͬһ��tss


// ��ʾ�����nr �Ľ��̺š�����״̬���ں˶�ջ�����ֽ�������Լ����
void show_task (int nr)
{
	task_struct *p=task[nr];
	u32 *esp=(u32*)((u32)p+PAGE_SIZE);
	int i= 0, j = 4096 - sizeof (task_struct);

	printk ("%d: pid=%d, state=%d, ", nr, p->pid, p->state);
	while (i < j && !((char *) (p + 1))[i])	// ���ָ���������ݽṹ�Ժ����0 ���ֽ�����
		i++;
	printk ("%d (of %d) chars free in kernel stack\n", i, j);

	
	DEBUG("ss:%x esp:%x eip:%x cs:%x eip:%x ds:%x es:%x fs:%x edx:%x ecx:%x ebx:%x\n"
			"eip:%x ebp:%x esi:%x edi:%x eax:%x\n",*(esp-1),*(esp-2),*(esp-3),*(esp-4),\
			*(esp-5),*(esp-6),*(esp-7),*(esp-8),*(esp-9),*(esp-10),\
			*(esp-11),*(esp-12),*(esp-13),*(esp-14),*(esp-15),*(esp-16));	
}

// ��ʾ�������������š����̺š�����״̬���ں˶�ջ�����ֽ�������Լ����
void show_stat (void)
{
	int i;

	for (i = 0; i < NR_TASKS; i++)// NR_TASKS ��ϵͳ�����ɵ������̣�����������64 ������
		if (task[i])		// ������include/kernel/sched.h ��4 �С�
			show_task (i);
}


/*�л�����:
			0.��������״̬
			1.current ָ�������� 
			2.tss->esp0 ָ���������ں˶�ջ
			3.�л�ҳ��(cr3)
			4.�ָ�esp,eip
			5.�ָ�����״̬
*/
 void switch_to(int next)
 {
 	/* �����ֳ� */
 	asm("pusha\n"
		"pushf\n"
		"mov %%esp,%0\n"
		"movl $1f,%1\n"
		:"=m"(current->pause_stack),"=m"(current->pause_eip):);


	/* �л����� */
	current=task[next];		//current
	current->tss->esp0=(u32)(current)+PAGE_SIZE; //tss
	asm("mov %0,%%cr3\n"	//cr3
		"mov %1,%%esp\n"	//esp
		"jmp *%2\n"			//eip
		::"g"(current->cr3),"m"(current->pause_stack),"m"(current->pause_eip));
		

	/* �ָ��ֳ� */
	asm("1:popf\n"
		"popa\n");

 }


//�������
void schedule(void)
{
	int i,next,c;
	task_struct ** p=task;	

	/* ���alarm�����̵ı�����ʱֵ���������κ��ѵõ��źŵĿ��ж����� */
	for(i=0;i<NR_TASKS;i++){
		//����
		if(task[i] && task[i]->alarm && task[i]->alarm < jiffies){
			task[i]->signal |= (1 << (SIGALRM - 1));
			task[i]->alarm = 0;
			DEBUG("alarm: task%d\n",i);
		}
		//����з������źţ���������
		//Ŀǰ�����ѣ���Ϊ���Ѻ�Ӧ��longjmpֱ��ִ���źŲ�׽����������ԭ·���أ����ǣ�Ŀǰ��δʵ��
		/*
		if( task[i] && (task[i]->signal &~task[i]->blocked) && 
				task[i]->state==TASK_INTERRUPTIBLE ){
			task[i]->state = TASK_RUNNING;	//��Ϊ��������ִ�У�״̬
			DEBUG("signal: wakeup interruptible task%d\n",i);
		}*/
	}

	//�ҳ���Ӧ�õ��ȵ�����
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
		DEBUG("switch to%d\n",next);
		switch_to(next);
	}
}


int sys_pause(void)
{
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	return 0;
}

//��Ƕ���ѵ�ǰ����֮ǰ˯�ߵĽ���
void sleep_on(task_struct **p)
{
	task_struct *tmp;
	
	if(!p)
		return ;
	DEBUG("sleep_on:%d\n",current->pid);
	if(current==task[0])
		panic("task[0] trying to sleep");
	tmp = *p;
	*p = current;

	current->state = TASK_UNINTERRUPTIBLE;
	schedule();

	if(tmp)
		tmp->state = 0;
}


/* ��Զ�������±�˯�ߵĽ��� ����ʹ���ѹ����У����½��̱�˯�ˣ�*/
void interruptible_sleep_on(task_struct **p)
{
	task_struct *tmp;
	if(!p)
		return ;
	DEBUG("interruptible_sleep_on:%d\n",current->pid);
	if(current==task[0])
		panic("task[0] trying to sleep");
	tmp = *p;
	*p = current;
	
	while(1){		/* (*p && *p!=current) ʱ����˯�� */
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
	if(cpl)
		current->utime++;
	else
		current->stime++;
	
	current->counter--;
	if(!current->counter)
		if(cpl)
			schedule();
		else
			current->counter++;
}

//�������ӣ�����֮ǰ����ʣ��ʱ��
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

// ϵͳ���ù��� -- ���Ͷ�CPU ��ʹ������Ȩ�����˻�����?����
// Ӧ������increment ����0������Ļ�,��ʹ����Ȩ���󣡣�
int sys_nice (long increment)
{
	if (current->priority - increment > 0)
		current->priority -= increment;
	return 0;
}


void sched_init(void)
{	
	jiffies=0;//�ӿ��������ھ����ĵδ�������Ȼֻ��ʱ���жϿ��������Ч��������ǰ���ʼ����û��
	
	memset(task,0,sizeof(task));	
	current=task[0]=(task_struct *)(get_free_page()+VM_START);	
	memset(current,0,PAGE_SIZE);//Ϊ�˱����ڼ䣬���
	
	current->priority=10;
	current->counter=10;
	current->state=0;		//TASK_RUNNING
	current->tty=0;			//Ĭ��ttyͨ��0
	current->pid=0;			//���̺�
	current->father=0;
	current->uid=0;			//�����û�
	current->euid=0;
	current->gid=0;
	
	current->signal=0;
	current->blocked=0;
	
	current->start_time=jiffies;
	current->utime=0;
	current->stime=0;
	current->alarm=0;
	
	asm("mov %%cr3,%%eax\n"	//��ǰҳ����Ϊtask[0]��ҳ��
		"mov %%eax,%0"
		:"=m"(current->cr3)::"eax");

	//���н��̹���ͬһ��TSS	
	current->tss=&tss;
	current->tss->esp0=(unsigned long)current+PAGE_SIZE;
	current->tss->ss0=SELECTOR(KERNEL_DATA,PL_KERNEL);

	/*����0����ʼ��ջ��Ŀǰ��VM_START,Ӧ����Ϊcurrent+PAGE_SIZE��
	���ǣ�������Ҫ���ƶ�ջ������ݣ������޷����أ��������ö�ջ�ͷ���main()�����
	�������Ϳ���ֱ�Ӷ�����ջ����*/
	//current->esp=(unsigned long)current+PAGE_SIZE;
	
}
