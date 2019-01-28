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
	tty_init();				//终端
	init_vmalloc_area();	//系统堆		0xc0080000~0xc0090000
	
	init_gdt();				//段表
	mem_init();				//内存
	init_fat16();		//文件系统	
	
	init_page();		//页表
	sched_init();		//任务     //必须在页表初始化之前
		
	interrupt_init();		//中断
		
}


#include<ctype.h>



int sys_write(const char *s)
{
	printk("sys_write:%s\n",s);
}

void prb(unsigned char *addr)
{
	int x,y;
	
	for(x=0;x<16;x++)
		printk("%02x ",*addr++);
	printk("\n");

}


extern struct super_block * read_super(int dev);

void start_kernel()
{
	all_init();
	
	buffer_init(1*1024*1024+VM_START);
	unsigned long pos=1024;
	asm("mov $0x10,%%ax\n"
		"mov %%ax,%%fs"
		:::"eax");
	block_read(10,&pos,(char *)VM_START,512);
	prb((unsigned char*)VM_START);
	while(1);
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





