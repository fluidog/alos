#include<linux/sched.h>
#include<string.h>
#include<memory.h>
#include<linux/kernel.h>

int sys_fork();
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
	
	memcpy(newTask,current,PAGE_SIZE);//ǧ�������������棬��������ĸ�ֵ����Ч
	
	//��ʹnewTask->cr3ָ���µ�ҳĿ¼���õ��µ��û���ַ�ռ䣬�����ں˿ռ�
	if(copy_page_table(newTask)<0)	
		return -1;

	//���̱�ʶ��״̬
	newTask->pid=pid++;
	newTask->father=current->pid;
	newTask->leader=0;//����leader
	newTask->utime=newTask->stime=newTask->cutime=newTask->cstime=0;
	newTask->start_time=jiffies;

	//��ؽ��+1
	newTask->pwd->i_count++;
	newTask->root->i_count++;
	newTask->executable->i_count++;
	//����close_on_exec��־�����ƴ��ļ�
	for(i=0;i<NR_OPEN;i++){
		if( current->filp[i] ){
			if( (current->close_on_exec>>i)&0x1 ){ 	//�ر�
				newTask->filp[i]=NULL;
			}
			else{	//����
				newTask->filp[i]->f_count++;
			}		
		}		
	}
	return ret;
}

//�����ӽ��̶ϵ�״̬(pause_eip,pause_stack)
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
	//�ӽ��̷���ֵ0
	asm("movl $0,%0"
		:"=m"(*((u32 *)esp+1)):);

	//�����̷����ӽ���pid
	next=task[next]->pid;
	schedule();//���ӽ���������
}  


