/*
	此程序包含两个模块，fat16文件系统（fat.c）、段页内存系统（memory.c）。
	作用是：加载操作系统程序到0x1000处
	并使能页表0xc0000000~0xc0400000 -- 0~4M 和 0~4M -- 0~4M的映射关系。
		并跳转到操作系统入口（0xc0001000）执行。
*/
asm("jmp main");

#include<fat.h>
#include<ext2.h>
#include<memory.h>
#include<vga.h>

//#define KERNEL_VIR_ADDR		
#define KERNEL_LOAD_ADDR	0xc0001000
#define KERNEL_FILE_NAME_FAT	"ALOS       "
#define KERNEL_FILE_NAME		"alos"
char buffer0[1024];//2K缓冲区 BSS段，不占用文件大小
char buffer1[1024];

void main()
{
	vga_init();
	printf("loading  %s...\n",KERNEL_FILE_NAME);
	init_gdt();
	EN_GDT;
	init_page();
	EN_PAGE;
	
	//先默认操作系统在ext2文件中，失败便尝试fat文件系统
	if(ext2_load_file((void *)KERNEL_LOAD_ADDR,KERNEL_FILE_NAME)<0)
		fat_load_file((void *)KERNEL_LOAD_ADDR,KERNEL_FILE_NAME_FAT);

	//跳转到内核start:0xc0001000
	printf("start %s:0x%x\n");
	asm("mov %0,%%ax\n"
		"mov %%ax,%%ss\n"
		"mov %2,%%esp\n"
		"ljmp %1,%2\n"
		::"i"(selector(KERNEL_DATA,PL_KERNEL)),
		"i"(selector(KERNEL_CODE,PL_KERNEL)),
		"i"(KERNEL_LOAD_ADDR));
}
