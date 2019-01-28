#include<linux/sched.h>
#include<string.h>
#include<memory.h>
#include<linux/kernel.h>
int sys_fork()
{	
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
	

	u32 off;
	asm("mov %%esp,%0\n"
		:"=m"(off):);
	off-=(u32)current;
	newTask->pause_stack=(u32)newTask+off;

	asm("mov %%ebp,%0\n"
		:"=m"(off):);
	off-=(u32)current;

	asm("push %%eax\n"
		"mov %%esp,%%eax\n"
		"mov %1,%%esp\n"
		"push %%esi\n"
		"push %%edi\n"
		"push %2\n"
		"mov %%esp,%0\n"
		"mov %%eax,%%esp\n"
		"pop %%eax\n"
		:"=m"(newTask->pause_stack)
		:"m"(newTask->pause_stack),"g"((u32)newTask+off)/*ebp*/
		:"eax");
	
		
	asm("movl $1f,%0\n"
		:"=m"(newTask->pause_eip));
	
	
	asm("mov $0x111,%eax\n"
		"leave\n"
		"ret\n");
		
	asm("1:pop %ebp\n"
		"pop %edi\n"
		"pop %esi\n"
		"mov $0,%eax\n"
		"leave\n"
		"ret\n");
}


