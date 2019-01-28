/*
	此程序包含两个模块，fat16文件系统（fat.c）、段页内存系统（memory.c）。
	作用是：加载操作系统程序到0x1000处，并使能页表0xc0000000~0xc0400000 -- 0x0~0x4M的映射关系。
			并跳转到操作系统处（0xc0001000）执行。
*/
asm("jmp main");

#include<fat.h>
#include<memory.h>

#define KERNEL_VIR_ADDR		0xc0001000
#define KERNEL_LOAD_ADDR	0x1000
#define KERNEL_FILE_NAME	"ALOS       "


void main()
{
	init_gdt();
	init_page();
	load_file((void *)KERNEL_LOAD_ADDR,KERNEL_FILE_NAME);
	EN_PAGE;
	EN_GDT;	//enable gdt and long jmp to CS:KERNEL_VIR_ADDR
}