#include<memory.h>
#include<asm/system.h>
#include<malloc.h>
#include<string.h>
#include<linux/kernel.h>

#include<linux/sched.h>

static s8 put_page(pde_t* pdeHander,u32 virAddr,u32 phyAddr,u32 attr);

#include<fat.h>

#define EN_PAGE(cr3) do{\
	asm("mov %0,%%eax\n"	\
		"mov %%eax,%%cr3\n"	\
		"mov %%cr0,%%eax\n"	\
		"or $0x80000000,%%eax\n"	 \
		"mov %%eax,%%cr0"::"g"(cr3));\
}while(0)


void page_fault(int errorcode,u32 ip,u16 seg)
{
	printf("errorcode:%d seg:0x%x ip:0x%x\n",errorcode,seg,ip);
	pde_t * pdeHander=(pde_t *)(current->cr3+VM_START);
	u32 virAddr;
	asm("mov %%cr2,%0":"=g"(virAddr));	
	printf("fault adddr:0x%x\n",virAddr);
	u32 phyAddr=get_free_page();
	put_page(pdeHander,virAddr,phyAddr,USER_P_ATTR);

	int ret;
	printf("filename:%s\n",current->file_name);
	
	if(virAddr&0xfffff000<current->start_code)
		ret=file_fat_read(current->file_name, (void *)current->start_code, \
				0, PAGE_SIZE-(current->start_code-(virAddr&0xfffff000)));
	else
		ret=file_fat_read(current->file_name, (void*)(virAddr&0xfffff000),\
			(virAddr&0xfffff000)-current->start_code,PAGE_SIZE);

	if(ret){
		printf("ret:%d page fault!(cant find file)",ret);
		while(1);
	}
}


void init_page()
{
	pde_t * pde=(pde_t *)malloc(PAGE_SIZE);
	int i;
	for(i=0;i<1024;i++) pde[i]=0;
	u32 virAddr,phyAddr;
	/*attr:kernel V:0xc0000000~0xc1000000 -- P:0x0~0x1000000*/
	for(i=0,virAddr=VM_START,phyAddr=0;i<1024*4;i++)
	{
		put_page(pde,virAddr,phyAddr,KERNEL_P_ATTR);
		virAddr+=PAGE_SIZE; //4K
		phyAddr+=PAGE_SIZE;
	}

	/*attr:user v:0~4M -- P:0~4M  */
	/*for(i=0,virAddr=0,phyAddr=0;i<1024;i++)
	{
		put_page(pde,virAddr,phyAddr,USER_P_ATTR);
		virAddr+=PAGE_SIZE; //4K
		phyAddr=virAddr;
	}*/
	
	EN_PAGE((u32)pde-VM_START); //物理地址
	current->cr3=(u32)pde-VM_START;

}

/* 填充页表和页目录，对应virtual addr和physical addr(4K)*/
#define ERROR_UALIGN	-2
#define ERROR_USUPPORT	-3
static s8 put_page(pde_t* pdeHander,u32 virAddr,u32 phyAddr,u32 attr)
{	
	pte_t *pteHander;
	if(pdeHander[virAddr>>22]&0x1){		//存在对应页表			
		pteHander=(pte_t *)((u32)(pdeHander[virAddr>>22]&0xfffff000)+VM_START);//虚拟地址(线性地址)
	}else{	//申请一页内存作为页表
		if(attr&0x4)	//用户态页表
			pteHander=(pte_t*)(get_free_page()+VM_START);
		else			//内核态页表
			pteHander=(pte_t *)malloc(PAGE_SIZE);

		memset(pteHander,0,PAGE_SIZE);
		pdeHander[virAddr>>22]=(pde_t)( ((u32)pteHander)-VM_START | attr );//物理地址
	}
	pteHander[(virAddr>>12)&0x3ff]=(pte_t)(phyAddr&0xfffff000 | attr);
	return 0;
}

int copy_kernel_page_table(task_struct *newTask)
{
	u32 startAddr=VM_START;
	u32 size=16*1024*1024;//16MB内空间
	pde_t *pdeHander_new,*pdeHander_old;
	newTask->cr3=get_free_page();
	if((u32)(pdeHander_new=(pde_t *)(newTask->cr3+VM_START))==VM_START)
		return -1;
	memset(pdeHander_new,0,PAGE_SIZE);
	pdeHander_old=(pde_t *)(current->cr3+VM_START);

	while(startAddr<VM_START+size){
		pdeHander_new[startAddr>>22]=pdeHander_old[startAddr>>22];
		startAddr+=4*1024*1024;
	}
}
/*	将当前进程地址空间(startAddr~startAddr+size)，复制到新的进程()中*/
int copy_page_table(task_struct *newTask,u32 startAddr,u32 size)
{
	u32 endAddr=startAddr+size;
	startAddr=startAddr/PAGE_SIZE*PAGE_SIZE;
	pde_t *pdeHander;

	if((u32)(pdeHander=(pde_t *)(newTask->cr3+VM_START))==VM_START)
		return -1;

	/*while(startAddr<endAddr){
			
	if(pdeHander[startAddr>>22]&0x1){		//存在对应页表			
		pteHander=(pte_t *)((u32)(pdeHander[startAddr>>22]&0xfffff000)+VM_START);//虚拟地址(线性地址)
	}else{
		pteHander=(pte_t*)(get_free_page()+VM_START);
		memset(pteHander,0,PAGE_SIZE);
	}
	
	u32 new_page=get_free_page();//申请新页
	pteHander[(startAddr>>12)&0x3ff]=new_page|USER_P_ATTR;
	
	memcpy((void *)(new_page+VM_START),(void *)startAddr,PAGE_SIZE);//复制页内容
	startAddr+=4*1024;

	}*/
	while(startAddr<endAddr){
		u32 phyAddr;
		if((phyAddr=get_free_page())==0)
			return -1;
		memcpy((void *)(phyAddr+VM_START),(void *)startAddr,PAGE_SIZE);//复制页内容
		put_page(pdeHander,startAddr,phyAddr,USER_PT_ATTR);
		startAddr+=PAGE_SIZE;
	}
}

void display_page_table(u32 cr3)
{
	pde_t *pdeHander=(pde_t *)(cr3+VM_START);

	int pde_i,pte_j;
	for(pde_i=0;pde_i<1024;pde_i++){
		if(pdeHander[pde_i]&0x1){
			printf("V:0x%x~0x%x\n",pde_i<<22,(pde_i<<22)+4*1024*1024);
		}

	}
}
