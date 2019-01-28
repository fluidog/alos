#ifndef _MM_H
#define _MM_H

#include<sys/types.h>
#include<linux/kernel.h>
#include<linux/sched.h>
#include<linux/config.h>
#define ADDR2MAP(addr) (((addr)-LOW_MEM)/PAGE_SIZE)	// 物理内存 -> 页号
#define MAP2ADDR(page)	(((page)*PAGE_SIZE)+LOW_MEM)	//页号 -> 物理内存

char mem_map [ PAGING_PAGES ] ;

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
int copy_page_table(task_struct *newTask);

void verify_area (void *addr, int size);

void mem_init();
u32 get_free_page (void);
void free_page (u32 addr);
void display_mm();

#define en_page(cr3) do{\
	asm("mov %0,%%eax\n"	\
		"mov %%eax,%%cr3\n"	\
		"mov %%cr0,%%eax\n"	\
		"or $0x80000000,%%eax\n"\
		"mov %%eax,%%cr0"	\
		::"g"(cr3):"eax");\
}while(0)


/*attr about page*/
#define P_ATTR_KERNEL_RDWR		0x003	//可读写、内核
#define P_ATTR_KERNEL_RD		0x001	//只读、内核

#define P_ATTR_USER_RDONLY		0x005	//只读、用户
#define P_ATTR_USER_RDWR		0x007	//读写、用户

#endif

	

