#include<memory.h>
#include<asm/system.h>
#include<types.h>
static gdt_t gdt[GDT_NUM];
static gdtr_t gdtr={GDT_NUM*GDT_LEN,(u32)gdt};

extern struct tss_struct tss;
	
/*	function	*/
static s8 set_page(u32 virAddr,u32 phyAddr);


void init_gdt()
{
	((u64 *)gdt)[0]=0;
	//kernel  segment
	_set_seg_desc(gdt+KERNEL_CODE,SEG_TYPE_CODE,PL_KERNEL,0x0,0x000fffff); 
	_set_seg_desc(gdt+KERNEL_DATA,SEG_TYPE_DATA,PL_KERNEL,0x0,0x000fffff);

	//user segment
	_set_seg_desc(gdt+USER_CODE,SEG_TYPE_CODE,PL_USER,0x0,0x000fffff);
	_set_seg_desc(gdt+USER_DATA,SEG_TYPE_DATA,PL_USER,0x0,0x000fffff);

	u32 addr=(u32)&tss;
	_set_seg_tss(gdt+TSS,SEG_TYPE_TSS,PL_USER,addr,0x000fffff);

	//¼ÓÔØgdtr ltr
	asm("lgdt %0\n"
		"ltr %w1"
		::"m"(gdtr),"a"(SELECTOR(TSS,0)));
	//¸üÐÂ»º´æ 
	asm("mov %0,%%ax\n" 	
		"mov %%ax,%%ds\n"	
		"mov %%ax,%%ss\n"
		"mov %%ax,%%es\n"
		"mov %2,%%ax\n"
		"mov %%ax,%%fs\n"
		"push %1\n"
		"push $1f\n"
		"lret \n1:"
		::"i"(SELECTOR(KERNEL_DATA,PL_KERNEL)),
		"i"(SELECTOR(KERNEL_CODE,PL_KERNEL)),
		"i"(SELECTOR(USER_DATA,PL_USER))); 
}

