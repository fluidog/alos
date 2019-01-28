#ifndef _MM_H
#define _MM_H

#include<sys/types.h>
#include<linux/kernel.h>
#include<linux/sched.h>
#include<linux/config.h>

enum {
	NULL_DES=0,
	KERNEL_CODE,
	KERNEL_DATA,
	USER_CODE,
	USER_DATA,
	TSS,
	GDT_NUM
}; 
#define PL_KERNEL	0
#define PL_DRIVER	1
#define PL_SYSTEM	2
#define PL_USER		3

#define SELECTOR(segment,rpl) ( (segment)<<3 | (rpl&0x3) )

void init_gdt();
void init_page();
void display_page_table(u32 cr3);
int copy_kernel_page_table(task_struct *newTask);
int copy_page_table(task_struct *newTask,u32 startAddr,u32 size);

void mem_init();
u32 get_free_page (void);
int free_page (unsigned long addr);
void display_mm();

#define EN_PAGE(cr3) do{\
	asm("mov %0,%%eax\n"	\
		"mov %%eax,%%cr3\n"	\
		"mov %%cr0,%%eax\n"	\
		"or $0x80000000,%%eax\n"	 \
		"mov %%eax,%%cr0"::"g"(cr3):"eax");\
}while(0)


#endif

	

