#include<asm/system.h>
#include<types.h>

struct tss_struct tss;

static struct {
	u16 limit_low;
	u8 base_low[3];
	u16 access;
	u8 base_high;
} __attribute__ ((packed))gdt[GDT_NUM];

static struct{
	u16 limit;
	u32 gdt_baseAddr;
}__attribute__((packed))gdtr;

enum {
	NULL_DES=0,
	KERNEL_CODE,
	KERNEL_DATA,
	USER_CODE,
	USER_DATA,
	TSS,
	GDT_NUM
}; 
/*attr about segment*/
#define SEG_TYPE_DATA		0x2
#define SEG_TYPE_CODE		0xa
#define	SEG_TYPE_TSS		0x9
#define PL_KERNEL	0
#define PL_DRIVER	1
#define PL_SYSTEM	2
#define PL_USER		3
#define SELECTOR(segment,rpl) ( (segment)<<3 | (rpl&0x3) )




void init_gdt()
{
	//初始化gdtr
	gdtr.limit=GDT_NUM*8;
	gdtr.gdt_baseAddr=(u32)gdt;

	//NULL
	((u64 *)gdt)[0]=0;
	//kernel  segment
	_set_seg_desc(gdt+KERNEL_CODE,SEG_TYPE_CODE,PL_KERNEL,0x0,0x000fffff); 
	_set_seg_desc(gdt+KERNEL_DATA,SEG_TYPE_DATA,PL_KERNEL,0x0,0x000fffff);
	//user segment
	_set_seg_desc(gdt+USER_CODE,SEG_TYPE_CODE,PL_USER,0x0,0x000fffff);
	_set_seg_desc(gdt+USER_DATA,SEG_TYPE_DATA,PL_USER,0x0,0x000fffff);
	//tss
	_set_seg_tss(gdt+TSS,SEG_TYPE_TSS,PL_USER,&tss,0x000fffff);

	//加载gdtr ltr
	asm("lgdt %0\n"
		"ltr %w1"
		::"m"(gdtr),"a"(SELECTOR(TSS,0)));
	
	//更新缓存 
	asm("mov %0,%%ax\n" 	
		"mov %%ax,%%ds\n"
		"mov %%ax,%%es\n"
		"mov %%ax,%%ss\n"
		"mov %2,%%ax\n"
		"mov %%ax,%%fs\n"
		"push %1\n"
		"push $1f\n"
		"lret \n1:"
		::"i"(SELECTOR(KERNEL_DATA,PL_KERNEL)),
		"i"(SELECTOR(KERNEL_CODE,PL_KERNEL)),
		"i"(SELECTOR(USER_DATA,PL_USER))); 
}

