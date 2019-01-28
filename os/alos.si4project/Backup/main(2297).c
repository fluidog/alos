asm("jmp start_kernel");
#include<linux/config.h>
#include<memory.h>
#include<string.h>
#include<linux/kernel.h>
#include<malloc.h>
#include<asm/interrupt.h>
#include<linux/tty.h>
#include<fat.h>
#include<linux/sched.h>
#include<sys/stat.h>
#include<unistd.h>
#include<linux/sys.h>

#include<linux/fs.h>

#include<fcntl.h>
#include<drivers/ide.h>
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
	buffer_init(1*1024*1024+VM_START);	//缓冲区
}




void start_kernel()
{
	const char *name;
	int namelen;
	m_inode *inode;
	int fd;
	ext2_dir_entry_2 * en;
	buffer_head *bh;
	all_init();

	ROOT_DEV=IDE_DRIVER;
	
	asm("mov $0x10,%%ax\n"
		"mov %%ax,%%fs"
		:::"eax");
	
	mount_root();

	fd=sys_mknod("/dev0",0777|S_IFBLK,IDE_DRIVER);
	printk("fd:%d\n",fd);
	sys_sync();
	while(1);


	
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





