#include<memory.h>
#include<asm/system.h>
/*	data	*/
GDT gdt[GDT_NUM];
gdtr_t gdtr={GDT_NUM*GDT_LEN,(u32)gdt};
	
page_desc_t *page_desc=(page_desc_t *)PAGE_DESC_BASE;
/*	function	haha*/
static s8 set_page(u32 virAddr,u32 phyAddr);
void init_gdt()
{
	gdt[0]=0;
	_set_seg_desc(gdt+KERNEL_CODE,ATTR_RX,PL_KERNEL,0x0,0x000fffff); //kernel code segment
	_set_seg_desc(gdt+KERNEL_DATA,ATTR_RW,PL_KERNEL,0x0,0x000fffff);
		
}
void init_page()
{
	int i;
	for(i=0;i<1024;i++)
	{
		page_desc->pde[i]=0;
		page_desc->pte0[i]=0;
		page_desc->pte1[i]=0;
	}
	_set_pde(page_desc->pde+(0x0>>22),page_desc->pte0);
	_set_pde(page_desc->pde+(0xc0000000>>22),page_desc->pte1);
	u32 virAddr,phyAddr;
	/* v:0~4M -- P:0~4M*/
	for(i=0,virAddr=0,phyAddr=0;i<1024;i++)
	{
		set_page(virAddr,phyAddr);
		virAddr+=0x1000; //4K
		phyAddr=virAddr;
	}
	/* V:0xc0000000~0xc0400000 -- P:0x0~0x400000*/
	for(i=0,virAddr=0xc0000000,phyAddr=0;i<1024;i++)
	{
		set_page(virAddr,phyAddr);
		virAddr+=0x1000; //4K
		phyAddr+=0x1000;
	}
}

/* 填充页表和页目录，对应virtual addr和physical addr */
#define ERROR_UALIGN	-2
#define ERROR_USUPPORT	-3
static s8 set_page(u32 virAddr,u32 phyAddr)
{
	if((virAddr|phyAddr)&0x0fff)return ERROR_UALIGN;
	pde_t tmp_pde=(page_desc->pde[(virAddr>>22)&0x3ff])&0xfffff000;
	pte_t *tmp_pte_p=(pte_t *)tmp_pde+((virAddr>>12)&0x3ff);
	
	if( (tmp_pde==(u32)(page_desc->pte0))| \
			(tmp_pde==(u32)(page_desc->pte1)) \
		) _set_pte(tmp_pte_p,phyAddr);
	else	return ERROR_USUPPORT;
			
		return 0;
}
