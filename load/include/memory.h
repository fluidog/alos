#include<types.h>

void init_page();
void init_gdt();

/*attr about segment*/

typedef u64 GDT;
enum { 
	KERNEL_CODE=1,
	KERNEL_DATA,
	GDT_NUM
};
#define GDT_LEN	8  //length by byte per dpl
typedef struct{
	u16 limit;
	u32 gdt_baseAddr;
}__attribute__((packed))gdtr_t;
	
extern gdtr_t gdtr;

#define ATTR_RW		0x2
#define ATTR_RX		0xa
#define ATTR_RXC	0xe				//C means "conforming code segment"

#define PL_KERNEL	0
#define PL_USER		3

#define selector(segment,rpl) ( (segment)<<3 | (rpl&0x3) )

/*attr about page*/
#define KERNEL_P_ATTR	0x003
#define PAGE_DESC_BASE 0x90000



#define EN_GDT	do{		\
	asm("lgdt %0"::"m"(gdtr)); 	\
	asm("mov %0,%%ax\n" 	\
		"mov %%ax,%%ds\n"	\
		"mov %%ax,%%es\n"	\
		"mov %%ax,%%fs\n"	\
		"ljmp %1,$1f\n1:"	\
		::"i"(selector(KERNEL_DATA,PL_KERNEL)),\
		"i"(selector(KERNEL_CODE,PL_KERNEL))); \
}while(0)

#define EN_PAGE do{\
	asm("mov %0,%%eax\n"	\
		"mov %%eax,%%cr3\n"	\
		"mov %%cr0,%%eax\n"	\
		"or $0x80000000,%%eax\n"	\
		"mov %%eax,%%cr0"		\
		::"i"(PAGE_DESC_BASE):);\
}while(0)

	
