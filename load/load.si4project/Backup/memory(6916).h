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
	
extern GDT gdt[GDT_NUM];
extern gdtr_t gdtr;


#define ATTR_RW		0x2
#define ATTR_RX		0xa
#define ATTR_RXC	0xe				//C means "conforming code segment"

#define PL_KERNEL	0
#define PL_USER		3

#define selector(segment,rpl) ( (segment)<<3 | (rpl&0x3) )

/*attr about page*/
#define KERNEL_PT_ATTR	0x003
#define KERNEL_P_ATTR	0x003
#define PAGE_DESC_BASE 0x90000




#define EN_GDT	do{		\
	asm("lgdt %0"::"m"(gdtr)); 	\
	asm("mov %0,%%ax\n" 	\
		"mov %%ax,%%ds\n"	\
		"mov %%ax,%%es\n"	\
		"mov %%ax,%%ss\n"	\
		"mov %2,%%esp\n"	\
		"ljmp %1,%2\n"		\
		::"i"(selector(KERNEL_DATA,PL_KERNEL)),\
		"i"(selector(KERNEL_CODE,PL_KERNEL)),\
		"i"(KERNEL_VIR_ADDR):); \
}while(0)

#define EN_PAGE do{\
	asm("mov %0,%%eax\n"	\
		"mov %%eax,%%cr3\n"	\
		"mov %%cr0,%%eax\n"	\
		"or $0x80000000,%%eax\n"	\
		"mov %%eax,%%cr0"		\
		::"i"(PAGE_DESC_BASE):);\
}while(0)

#define _set_pde(pdeAddr,ptBase) \
	*(u32 *)(pdeAddr)=(KERNEL_PT_ATTR) | \
	((u32)(ptBase) & (0xfffff000))

#define _set_pte(pteAddr,pBase) \
	*(u32 *)(pteAddr)=(KERNEL_P_ATTR) | \
	((u32)(pBase) & (0xfffff000))


typedef u32 pde_t;
typedef u32 pte_t; 	
typedef struct{
	pde_t pde[1024];
	pte_t pte0[1024];
	pte_t pte1[1024];
	}page_desc_t;
	

