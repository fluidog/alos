asm("jmp main");

#include<fat.h>
#include<memory.h>

#define KERNEL_VIR_ADDR	0xc0001000
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