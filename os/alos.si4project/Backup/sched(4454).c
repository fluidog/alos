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

long volatile jiffies=0;	//ϵͳ�ӿ��������ĵδ���
long startup_time;	//��cmos_init�г�ʼ��
task_struct *current;
task_struct * task[NR_TASKS];
long pid=0;		//�����½��̵�pid��ÿ��+1
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

	
	DEBUG("ss:%x esp:%x eip:%x cs:%x eip:%x ds:%x es:%x fs:%x edx:%x ecx:%x ebx:%x "
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
			5.�ָ�����״̬*/
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
	return ;
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
		//DEBUG("switch to%d\n",next);
		switch_to(next);
	}
}

//˯��ֱ�����źŻ���
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
	//DEBUG("sleep_on:%d\n",current->pid);
	
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
	//DEBUG("interruptible_sleep_on:%d\n",current->pid);

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
	pid=0;	
	
	memset(task,0,sizeof(task));	
	current=task[0]=(task_struct *)(get_free_page()+VM_START);	
	memset(current,0,PAGE_SIZE);

	//�кܶ඼��Ĭ�ϵ�0�����Բ����ٴθ�0������Ϊ�����ֽ������е����ݣ�����Ҳд������
	current->state=TASK_RUNNING;
	current->priority=20;
	current->counter=20;
	current->signal=0;	//���ź�
	//sigaction[i]=0; //Ĭ��
	current->blocked=0;	//������
	
	
	//exit_code,start_code,end_code,end_data,brk,start_stack,mem_szie��execve�˽���ʱ����
	current->pgrp=current->father=current->pid=pid++;				//���̺�
	current->leader=current->session=0;
	current->suid=current->euid=current->uid=0;		//�û���
	current->gid=current->egid=current->sgid=current->uid;	//�û����
	
	current->alarm=0;	//������
	current->cstime=current->cutime=current->utime=current->stime=0;
	current->start_time=jiffies;
	
	current->tty=-1;	//Ĭ����tty�ն�
	current->umask=0;
	//pwd,root,executable�ڹ��ظ��ļ���execve�˽���ʱ����
	current->close_on_exec=0;	//Ĭ�ϲ��ر�
	//filp[NR_OPEN]=0;	//Ĭ��
	
	//���н��̹���ͬһ��TSS	
	current->tss=&tss;
	current->tss->esp0=(unsigned long)current+PAGE_SIZE;
	current->tss->ss0=SELECTOR(KERNEL_DATA,PL_KERNEL);
	asm("mov %%cr3,%%eax\n"	//��ǰҳ����Ϊtask[0]��ҳ��
		"mov %%eax,%0"
		:"=m"(current->cr3)::"eax");
	
	/*����0����ʼ��ջ��Ŀǰ��VM_START,Ӧ����Ϊcurrent+PAGE_SIZE��
	���ǣ�������Ҫ���ƶ�ջ������ݣ������޷����أ��������ö�ջ�ͷ���main()�����
	�������Ϳ���ֱ�Ӷ�����ջ����*/
	//esp=(unsigned long)current+PAGE_SIZE;	
}
