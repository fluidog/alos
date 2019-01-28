#ifndef _MM_H
#define _MM_H

#include<types.h>
#include<linux/sched.h>

//unsigned long get_free_page (void);
//unsigned long put_page (unsigned long page, unsigned long address);
//void free_page (unsigned long addr);
void init_page();
void init_gdt();

void mem_init();
unsigned long get_free_page (void);
void free_page (unsigned long addr);
void display_page_table(u32 cr3);
int copy_kernel_page_table(task_struct *newTask);
int copy_page_table(task_struct *newTask,u32 startAddr,u32 size);


void display_mm();


#define VM_START	0xc0000000
#define VM_END		0xc1000000
#define PAGE_SIZE 	4096		// 定义内存页面的大小(字节数)。



/*attr about segment*/

typedef struct {
	u16 limit_low;
	u8 base_low[3];
	u16 access;
	u8 base_high;
} __attribute__ ((packed))gdt_t;

typedef struct{
	u16 limit;
	u32 gdt_baseAddr;
}__attribute__((packed))gdtr_t;
enum { 
	KERNEL_CODE=1,
	KERNEL_DATA,
	USER_CODE,
	USER_DATA,
	TSS,
	GDT_NUM
};
#define GDT_LEN	8  //length by byte 
#define SEG_TYPE_DATA		0x2
#define SEG_TYPE_CODE		0xa
#define	SEG_TYPE_TSS		0x9
#define PL_KERNEL	0
#define PL_DRIVER	1
#define PL_SYSTEM	2
#define PL_USER		3
#define selector(segment,rpl) ( (segment)<<3 | (rpl&0x3) )
	


typedef u32 pte_t; 
typedef u32 pde_t; 

	/*attr about page*/
#define KERNEL_PT_ATTR	0x003
#define KERNEL_P_ATTR	0x003
#define USER_PT_ATTR	0x007
#define USER_P_ATTR		0x007

typedef struct{
	u32 pid;
	u32 cr3;
}vmArea_desc_t;
extern vmArea_desc_t vmArea[0x10];

#endif

	

