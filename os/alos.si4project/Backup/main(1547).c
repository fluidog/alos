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

#include<unistd.h>
#include<linux/sys.h>

#include<linux/fs.h>

#include<fcntl.h>
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
	buffer_init(1*1024*1024+VM_START);	//������
}



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


void start_kernel()
{
	all_init();
	
	ROOT_DEV=0xa0;
	
	asm("mov $0x10,%%ax\n"
		"mov %%ax,%%fs"
		:::"eax");

	mount_root();
	int fd;
	if((fd=sys_open("/lost+found/test.c",O_RDONLY,0))<0)
		panic("open file");
	char buf[100]={0};
	int x=sys_read(fd,buf,100);
	printk("x=%d\n",x);
	printk("%s\n",buf);
	
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



void mm_test(int    * start,int *end)
{
	

}





