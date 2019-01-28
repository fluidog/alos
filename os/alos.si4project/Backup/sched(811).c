/*
 *  linux/kernel/sched.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * 'sched.c' is the main kernel file. It contains scheduling primitives
 * (sleep_on, wakeup, schedule etc) as well as a number of simple system
 * call functions (type getpid(), which just extracts a field from
 * current-task
 */
#include <linux/sched.h>
#include <linux/kernel.h>
#include <memory.h>
#include<string.h>
#include <asm/system.h>
#include <asm/io.h>

#include <signal.h>

#define _S(nr) (1<<((nr)-1))
#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))

void show_task(int nr)
{

}

void show_stat(void)
{

}

#define LATCH (1193180/HZ)

extern void mem_use(void);

extern int timer_interrupt(void);
extern int system_call(void);

union task_union {
	task_struct task;
	char stack[PAGE_SIZE];
};

//static union task_union init_task = {};

long volatile jiffies=0;
long startup_time=0;
task_struct *current;
task_struct *last_task_used_math = NULL;

task_struct * task[NR_TASKS];

long user_stack [ PAGE_SIZE>>2 ];

struct {
	long * a;
	short b;
	} stack_start = { & user_stack [PAGE_SIZE>>2] , 0x10 };
/*
 *  'math_state_restore()' saves the current math information in the
 * old math state array, and gets the new ones from the current task
 */
void math_state_restore()
{
	
}

struct tss_struct tss;

 #if 0
 static void switch_to(int next)
{

	if(task[next]==current)
		return ;
	asm("pusha\n"	//save reg
		"pushf\n"
		"mov %%esp,%0\n"	//当前任务esp
		:"=g"(current->pause_stack));
	

	current=task[next];
	current->tss->esp0=(u32)(task[next])+PAGE_SIZE;
	
	
	asm("mov %0,%%esp\n"	//目标任务esp
		"popf\n"	//load	reg
		"popa\n"
		::"g"(current->pause_stack));
	
	
	asm("mov  %%esp,%0\n":"=g"(current->pause_stack));
	
		asm("mov %0,%%cr3\n"		//cr3
		::"g"(current->cr3));

	/*if(current->pause_stack==(u32)current+PAGE_SIZE){
			asm("push %0\n"
			"push $0x400\n"
			"push %1\n"
			"push %2\n"
			"mov %0,%%eax\n"
			"mov %%ax,%%ds\n"
			"mov %%ax,%%es\n"
			"lret"
			::"i"(selector(USER_DATA,PL_USER)),
			"i"(selector(USER_CODE,PL_USER)),
			"g"(task1-VM_START));
		}*/
}


asm(".text\n.global switch_to\n"
	"switch_to:\n"
	"pusha\n"
	"pushf\n"
	"mov %%esp,%0\n"

	""

	:"")
#endif

/*
 static void switch_to(int next)
{
	printf("next is%d\n",next);
	if(task[next]==current)
		return ;
	asm("pusha\n"	//save reg
		"pushf\n"
		"mov %%esp,%0\n"//当前任务esp
		:"=g"(current->pause_stack));
	

	current=task[next];
	current->tss->esp0=(u32)(task[next])+PAGE_SIZE;
	
	

	//printf("p:%x  s:%x\n",current->pause_stack,(u32)current+PAGE_SIZE);
	asm("mov %0,%%esp\n"	//目标任务esp
		"popf\n"	//load	reg
		"popa\n"
		::"g"(current->pause_stack));
	
	
	asm("mov  %%esp,%0\n":"=g"(current->pause_stack));
	
		asm("mov %0,%%cr3\n"		//cr3
		::"g"(current->cr3));
		
	//printf("p:%x  s:%x\n",current->pause_stack,(u32)current+PAGE_SIZE);
	//while(1);
		if(current->pause_stack==(u32)current+PAGE_SIZE){
		
			printf("in\n");
				asm("push %0\n"
				"push $0x400\n"
				"push %1\n"
				"push %2\n"
				"mov %0,%%eax\n"
				//"mov %%ax,%%ds\n"
				//"mov %%ax,%%es\n"
				"lret"
				::"i"(selector(KERNEL_DATA,PL_KERNEL)),
				"i"(selector(KERNEL_CODE,PL_KERNEL)),
				"g"(task1));
			}
			
}
*/
 static void switch_to_restore();

 void switch_to_save(int next)
 {
 	asm("pusha\n"
		"pushf\n"
		"mov %%esp,%0\n"
		"movl $1f,%1\n"
		:"=m"(current->pause_stack),"=m"(current->pause_eip):);

	
	current=task[next];
	current->tss->esp0=(u32)(current)+PAGE_SIZE; //tss
	asm("mov %0,%%esp\n"	//esp
		"mov %1,%%cr3\n"	//cr3
		::"g"(current->pause_stack),"g"(current->cr3));
	
	//printf("eip:0x%x esp:0x%x cur:0x%x\n",current->pause_eip,current->pause_stack,(u32)current);
	//asm("lq:jmp lq\n");
	//恢复eip
	asm("jmp *%0\n"
		::"m"(current->pause_eip));

	asm("1:popf\n"
		"popa\n");

 }

 static void switch_to_restore()
 {
 	asm("popf\n"
		"popa\n");
 }


void schedule(void)
{
	int i,next,c;
	task_struct ** p=task;
	
	
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
	printf("schedule:%d\n",next);
	if(current!=task[next])
		switch_to_save(next);				
}


int sys_pause(void)
{
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	return 0;
}

void sleep_on(task_struct **p)
{

}

void interruptible_sleep_on(task_struct **p)
{

}

void wake_up(task_struct **p)
{
	if (p && *p) {
		(**p).state=0;
		*p=NULL;
	}
}

/*
 * OK, here are some floppy things that shouldn't be in the kernel
 * proper. They are here because the floppy needs a timer, and this
 * was the easiest way of doing it.
 */
static struct task_struct * wait_motor[4] = {NULL,NULL,NULL,NULL};
static int  mon_timer[4]={0,0,0,0};
static int moff_timer[4]={0,0,0,0};
unsigned char current_DOR = 0x0C;



#define TIME_REQUESTS 64

static struct timer_list {
	long jiffies;
	void (*fn)();
	struct timer_list * next;
} timer_list[TIME_REQUESTS], * next_timer = NULL;

void add_timer(long jiffies, void (*fn)(void))
{

}

void do_timer(void*data)
{
	current->counter--;	
	if(!current->counter)
		schedule();
}


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

int sys_nice(long increment)
{

}


void sched_init(void)
{
	task_struct *init_task,*second_task;
	int x;
	
	init_task=(task_struct *)(x=get_free_page()+VM_START);
	init_task->file_name="/BIN/TEST0.BIN";
	init_task->priority=10;
	init_task->counter=10;
	init_task->state=0;
	init_task->start_code=0;
	init_task->mem_size=0x1000;
	init_task->start_stack=0x400;

	init_task->tss=&tss;
	tss.esp0=(unsigned long)init_task+PAGE_SIZE;
	tss.ss0=selector(KERNEL_DATA,PL_KERNEL);
	
	current=task[0]=init_task;	

}
