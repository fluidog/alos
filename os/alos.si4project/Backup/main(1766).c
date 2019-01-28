asm("jmp start_kernel");
#include<linux/config.h>
#include<memory.h>
#include<linux/kernel.h>
#include<malloc.h>
#include<asm/interrupt.h>
#include<linux/tty.h>
#include<fat.h>
#include<linux/sched.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<linux/fs.h>
#include<linux/sys.h>
#include<fcntl.h>
#include<time.h>
extern void cmos_init(void);
void all_init(void)
{	
	tty_init();				//终端
	
	cmos_init();			//读取coms并设置开机time，内存，磁盘等
	
	init_vmalloc_area();	//系统堆		0xc0080000~0xc0090000
	
	init_gdt();				//段表
	
	mem_init();				//内存
	

	init_fat16();			//文件系统	
	init_page();		//页表
	
	sched_init();		//任务     //必须在页表初始化之前
		
	interrupt_init();		//中断
	
	buffer_init(1*1024*1024+VM_START);	//缓冲区
	ROOT_DEV=IDE_DRIVER;
	mount_root();	//文件系统
}

void pb(unsigned char *buf,int count)
{
	while(count--)
	{
		printk("%x ",*buf++);

	}
}

void start_kernel()
{
	int fd,error_code;
	
	all_init();
	

	/*下面函数需要从用户态进入，因为需要用fs取用户态数据，
	这里直接使fs为内核段，从而在内核态取数据并执行*/
	asm("mov $0x10,%%ax\n"
		"mov %%ax,%%fs"
		:::"eax");
	

	/*目前task[0]的堆栈起始为VM_START,但是为了内核态直接支持fork(),因此，
		将堆栈设为current+PAGE_SIZE*/
	asm("mov %0,%%esp"::"g"((unsigned long)current+PAGE_SIZE));
	
		//建立基本的设备文件	
	error_code=sys_mkdir("/dev", 0755);	//读、进入
	if(error_code && error_code!=-EEXIST)
		panic("mkdir /dev error");

	error_code=sys_mknod("/dev/tty0",S_IFCHR|0666,0x0400);//0号终端设备 (字符设备)
	if(error_code && error_code!=-EEXIST)
		panic("mknod /dev/tty0 error");

	error_code=sys_mknod("/dev/tty1",S_IFCHR|0666,0x0401);//1号终端设备,用于串口，暂时未实现
	if(error_code && error_code!=-EEXIST)
		panic("mknod /dev/tty1 error");
	
	error_code=sys_mknod("/dev/tty",S_IFCHR|0666,0x0500);//默认终端设备,打开需要进程拥有默认终端
	if(error_code && error_code!=-EEXIST)
		panic("mknod /dev/tty error");
		
	sys_setsid();
	
	//打开stdio
	if((fd=sys_open("/dev/tty0",O_RDWR,0))<0)	//stdin	0
		panic("open stdin error");
	if((fd=sys_dup(fd))<0)
		panic("open stdout error");
	if((fd=sys_dup(fd))<0)
		panic("open stderr error");	
	
	//sys_sync();
	asm("sub $8,%%esp\n"	//ss,esp，之后填充
		//系统调用execve,不直接调用sys_execve的原因是可以直接返回
		"mov %0,%%eax\n"	//execve
		"mov %1,%%ebx\n"	//filename
		"mov $0,%%ecx\n"	//argv
		"mov $0,%%edx\n"	//envp
		"int $0x80\n"
		::"i"(__NR_execve),
		"i"("/bin/shell"));

	panic("shell run error");
}







