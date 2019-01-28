#include<linux/sched.h>
#include<string.h>
#include<memory.h>
#include<linux/kernel.h>

asm(".text\n.global sys_fork\n"
		"sys_fork:"
		"push %ebp\n"	//����״̬
		"push %esi\n"
		"push %edi\n"
		"call copy_process\n"//���ƽ������ݽṹ
		"push %eax\n"
		"push %esp\n"
		"call set_ret_stat\n"//�����ӽ��̶ϵ�esp��eip���������ӽ��̷���ֵ
		"add $4,%esp\n"		//�ӽ��̴Ӵ˽���
		"pop %eax\n"		//����ֵ
		"pop %edi\n"
		"pop %esi\n"
		"pop %ebp\n"
		"ret");

static int copy_process()
{	
	int i;
	for(i=0;i<NR_TASKS;i++){
		if(!task[i])
			break;
	}
	if(i==NR_TASKS)
		return -1;

	task_struct *newTask=NULL;
	if((newTask=(task_struct *)(get_free_page()+VM_START))==(task_struct *)VM_START)
		return -1;
	task[i]=newTask;

	memcpy(newTask,current,PAGE_SIZE);	
	copy_kernel_page_table(newTask);
	copy_page_table(newTask,current->start_code,current->start_stack-current->start_code);
	return i;
}

static void set_ret_stat(u32 esp,int next){
	if(next<0)
		return ;
	
	//�ӽ��̶ϵ�ip
	asm("mov %1,%0"
		:"=g"(task[next]->pause_eip):"m"(*(&esp-1)));
	//�ӽ����ں˶�ջ
	esp=(u32)task[next]+esp%PAGE_SIZE-4;
	asm("mov %1,%0"
		:"=m"(task[next]->pause_stack):"g"(esp));
	//�ӽ��̷���ֵ
	asm("movl $0,%0"
		:"=m"(*((u32 *)esp+1)):);
}  


