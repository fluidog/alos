#include<linux/sched.h>
#include<string.h>
#include<memory.h>
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
	if((newTask=(task_struct *)get_free_page())==NULL)
		return -1;

	memcpy(newTask,current,PAGE_SIZE);
	newTask->cr3=0;
	copy_kernel_page_table(newTask);
	copy_page_table(newTask,current->start_code,current->mem_size);
	

	unsigned int off;
	asm("mov %%esp,%0\n"
		:"=g"(off):);
	off=off-(u32)current;
	asm("mov %1,%0\n"
		:"=g"(newTask->pause_stack):"g"(newTask+off));

		
	asm("mov 1f,%0\n"
		:"=g"(newTask->pause_eip));

	asm("mov 0x111,%eax\n"
		"leave\n"
		"ret\n");

	asm("1:mov 0,%eax\n"
		"leave\n"
		"ret\n");
}


