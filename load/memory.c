#include<memory.h>
#include<asm/system.h>
#include<string.h>
#include<vga.h>
/*	data	*/
//两个段，一个内核代码段，一个内核数据段
GDT gdt[GDT_NUM];
gdtr_t gdtr={GDT_NUM*GDT_LEN,(u32)gdt};
//一个页目录表，两个页表
typedef struct page_desc_t{
	u32 pde[1024];
	u32 pte0[1024];
	u32 pte1[1024];
	}*page_desc_t;
	
page_desc_t page_desc=(page_desc_t)PAGE_DESC_BASE;
	
/*	function*/

static void put_page(u32 cr3,u32 virAddr,u32 phyAddr)
{	
 	u32 *pde,*pte,tmp;
	
	pde=(u32*)cr3;//页目录表首地址
	if(!(pde[virAddr>>22] & 0x1)){//不存在对应页目录项
		printf("error:don`t have pte in:0x%x\n",virAddr);
		while(1);
	}
	
	pte=(u32 *)((pde[virAddr>>22] & 0xfffff000)); //页表首地址
	//页面已经映射,出错
	if(pte[(virAddr>>12)&0x3ff] & 0x1){	
		printf("waring:page have alread maped in:0x%x\n",virAddr);
		//while(1);
	}
	
	pte[(virAddr>>12)&0x3ff]=phyAddr&0xfffff000 | KERNEL_P_ATTR;
}


void init_gdt()
{
	gdt[0]=0;
	_set_seg_desc(gdt+KERNEL_CODE,ATTR_RX,PL_KERNEL,0x0,0x000fffff); //kernel code segment
	_set_seg_desc(gdt+KERNEL_DATA,ATTR_RW,PL_KERNEL,0x0,0x000fffff);
}
void init_page()
{
	int i;
	memset(page_desc->pde,0,1024);
	memset(page_desc->pte0,0,1024);
	memset(page_desc->pte1,0,1024);
	
	page_desc->pde[0>>22]=(u32)(&page_desc->pte0) | KERNEL_P_ATTR;
	page_desc->pde[0xc0000000>>22]=(u32)(&page_desc->pte1) | KERNEL_P_ATTR;
	
	u32 virAddr,phyAddr;
	/* v:0~4M -- P:0~4M*/
	for(i=0,virAddr=0,phyAddr=0;i<1024;i++)
	{
		put_page((u32)page_desc->pde,virAddr,phyAddr);
		virAddr+=0x1000; //4K
		phyAddr=virAddr;
	}
	/* V:0xc0000000~0xc0400000 -- P:0x0~0x400000*/
	for(i=0,virAddr=0xc0000000,phyAddr=0;i<1024;i++)
	{
		put_page((u32)page_desc->pde,virAddr,phyAddr);
		virAddr+=0x1000; //4K
		phyAddr+=0x1000;
	}
	printf("page table at:0x%x\n",(u32)page_desc->pde);
	printf("maped:0~4M -- 0~4M    0xc0000000~0xc0000000+4M -- 0~4M\n");
}




