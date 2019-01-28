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
extern void _keyboard_interrupt(void *ignore);


void all_init(void)
{
	tty_init();				//�ն�
	init_vmalloc_area();	//ϵͳ��		0xc0080000~0xc0090000
	interrupt_init();		//�ж�
	init_gdt();				//�α�
	mem_init();				//�ڴ�
	

	sched_init();		//����     //������ҳ���ʼ��֮ǰ
	init_page();		//ҳ��
	init_fat16();		//�ļ�ϵͳ
	
}
extern void do_timer(void *);

long ip=0xc0000200;
long addr=SELECTOR(KERNEL_CODE,PL_KERNEL);


#include<ctype.h>

void task0()
{
	asm("int $13");
}

int sys_write(const char *s)
{
	printf("sys_write:%s\n",s);
}


void start_kernel()
{
	all_init();
	sys_call_table[__NR_write]=(fn_ptr)sys_write;
	sys_call_table[__NR_fork]=(fn_ptr)sys_fork;

	irq_install_handler(0,do_timer,NULL);

	asm("push %0\n"
		"push $0x200\n"
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






