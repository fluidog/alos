#ifndef _MM_H
#define _MM_H

#include<types.h>
#include<linux/kernel.h>

#define VM_START	0xc0000000	//内核地址空间起始地址
#define PAGE_SIZE 	4096		//定义内存页面的大小(字节数)。

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

	

