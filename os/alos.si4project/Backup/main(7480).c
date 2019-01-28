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
#include<errno.h>
#include<string.h>
#include<linux/fs.h>
#include<linux/sys.h>
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
	ROOT_DEV=IDE_DRIVER;
	mount_root();	//�ļ�ϵͳ
}


void start_kernel()
{

	int fd,error_code;
	
	all_init();

	/*���溯����Ҫ���û�̬���룬��Ϊ��Ҫ��fsȡ�û�̬���ݣ�
	����ֱ��ʹfsΪ�ں˶Σ��Ӷ����ں�̬ȡ���ݲ�ִ��*/
	asm("mov $0x10,%%ax\n"
		"mov %%ax,%%fs"
		:::"eax");

	/*Ŀǰtask[0]�Ķ�ջ��ʼΪVM_START,����Ϊ���ں�ֱ̬��֧��fork(),��ˣ�
		����ջ��Ϊcurrent+PAGE_SIZE*/
	asm("mov %0,%%esp"::"g"((unsigned long)current+PAGE_SIZE));

	
	//�����������豸�ļ�	
	error_code=sys_mkdir("/dev", 0755);	//��������
	if(error_code && error_code!=-EEXIST)
		panic("mkdir /dev error");
	
	error_code=sys_mknod("/dev/tty0",S_IFCHR|0666,0x0400);//0���ն��豸 (�ַ��豸)
	if(error_code && error_code!=-EEXIST)
		panic("mknod /dev/tty0 error");

	error_code=sys_mknod("/dev/tty1",S_IFCHR|0666,0x0401);//1���ն��豸,���ڴ��ڣ���ʱδʵ��
	if(error_code && error_code!=-EEXIST)
		panic("mknod /dev/tty1 error");
	
	error_code=sys_mknod("/dev/tty",S_IFCHR|0666,0x0500);//Ĭ���ն��豸,����Ҫ����ӵ��Ĭ���ն�
	if(error_code && error_code!=-EEXIST)
		panic("mknod /dev/tty error");
	
	//��stdio
	if((fd=sys_open("/dev/tty0",O_RDONLY,0))<0)	//stdin	0
		panic("open stdin error");
	if((fd=sys_open("/dev/tty0",O_WRONLY,0))<0)	//stdout 1
		panic("open stdout error");
	if((fd=sys_open("/dev/tty0",O_WRONLY,0))<0)	//stderr 2
		panic("open stderr error");
	sys_sync();
	
	sys_execve("/bin/shell",NULL,NULL);
	panic("shell run error");
}





