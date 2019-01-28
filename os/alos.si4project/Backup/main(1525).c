asm("jmp start_kernel");

#include<memory.h>
#include<string.h>
#include<linux/kernel.h>
#include<malloc.h>
#include<asm/interrupt.h>
#include<linux/tty.h>
#include<fat.h>
#include<linux/sched.h>

#include<unistd.h>
#include<linux/sys.h>

#include<linux/fs.h>


void all_init(void)
{
	tty_init();				//�ն�
	init_vmalloc_area();	//ϵͳ��		0xc0080000~0xc0090000
	
	init_gdt();				//�α�
	mem_init();				//�ڴ�
	init_fat16();		//�ļ�ϵͳ	
	
	init_page();		//ҳ��
	sched_init();		//����     //������ҳ���ʼ��֮ǰ
		
	interrupt_init();		//�ж�
		
}


#include<ctype.h>



int sys_write(const char *s)
{
	printk("sys_write:%s\n",s);
}


void start_kernel()
{
	all_init();
	buffer_init(1*1024*1024+VM_START);
	unsigned long pos=0;
	asm("mov $0x10,%%ax\n"
		"mov %%ax,%%fs"
		:::"eax");
	block_read(0,&pos,(char *)VM_START,512);

	sys_call_table[__NR_write]=(fn_ptr)sys_write;
	sys_call_table[__NR_fork]=(fn_ptr)sys_fork;
	sys_call_table[__NR_read]=(fn_ptr)tty_read;
	
	asm("push %0\n"
		"push $0x500\n"
		"push %1\n"
		"push %2\n"
		"mov %0,%%eax\n"
		"mov %%ax,%%ds\n"
		"mov %%ax,%%es\n"
		"lret"
		::"i"(SELECTOR(USER_DATA,PL_USER)),
		"i"(SELECTOR(USER_CODE,PL_USER)),
		"g"(0));
}



void mm_test(int    * start,int *end)
{
	

}





