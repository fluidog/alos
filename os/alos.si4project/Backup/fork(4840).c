#include<linux/sched.h>
#include<string.h>
#include<memory.h>
#include<linux/kernel.h>
int sys_fork()
{
	printf("sys_fork\n");
	int i;
	for(i=0;i<NR_TASKS;i++){
		if(!task[i])
			break;
	}
	if(i==NR_TASKS)
		return -1;

	task_struct *newTask=NULL;
	if((newTask=(task_struct *)(get_free_page()+VM_START))==NULL)
		return -1;
	task[i]=newTask;
	
	memcpy(newTask,current,PAGE_SIZE);
	copy_kernel_page_table(newTask);
	copy_page_table(newTask,current->start_code,current->mem_size);
	

	unsigned int off;
	asm("mov %%esp,%0\n"
		:"=g"(off):);
	off=off-(u32)current;
	asm("mov %1,%0\n"
		:"=g"(newTask->pause_stack):"g"((int)newTask+off));

	
	
	
asm("push %%eax\n"
	"mov %%esp,%%eax\n"
	"mov %1,%%esp\n"
	"pusha\n"
	"pushf\n"
	"mov %%esp,%0\n"
	"mov %%eax,%%esp\n"
	"pop  %%eax\n"
	:"=m"(newTask->pause_stack)
	:"m"(newTask->pause_stack):"eax");
	//printf("stack:%x  task:%x\n",newTask->pause_stack,(u32)newTask);//asm("lq:jmp lq\n");

		
	asm("movl $1f,%0\n"
		:"=m"(newTask->pause_eip));
	
	
	asm("mov $0x111,%eax\n"
		"leave\n"
		"ret\n");

		
	asm("1:popf\n"
		"popa\n"
		"mov $0,%%eax\n"
		"mov %%ebp,%0\n"
		:"":


		"leave\n"
		"ret\n");
}


