#include<memory.h>
static int page_num;
u32  get_free_page (void)
{
	int back=page_num;
	do{
		if(mem_map[page_num++])
			continue;

		//找到空闲页
		mem_map[page_num-1]++;
		return MAP2ADDR(page_num-1);

	}while(page_num!=back);	
	
	return 0;//因为0地址不属于可分配内存，所以用于错误标志
}
 

void free_page (u32 phyAddr)
{
	if (phyAddr < LOW_MEM || phyAddr >= MEMORY)	//系统内核错误
		panic("try to free kernel or nonexistent page");
	
	if (mem_map[ADDR2MAP(phyAddr)]>0)
		mem_map[ADDR2MAP(phyAddr)]--;	
	
	panic("trying to free free page");
}


void mem_init(void)
{
	u32 i;
	for (i=0 ; i<PAGING_PAGES ; i++)
		mem_map[i] = 0;
	
}

void calc_mm(void)
{
	printk("/****************physical memory all used*******************/\n");
	u32 i,flag;
	for(i=0,flag=0;i<PAGING_PAGES;i++)
	{						
		if(mem_map[i] || flag){		
			if(mem_map[i] && !flag){
				printk("0x%x    ~    ",MAP2ADDR(i));
				flag=1;
			}
			if(!mem_map[i] && flag){
				printk("0x%x-1\n",MAP2ADDR(i));
				flag=0;
			}
		}
	}
	if(flag)
		printk("0x%x-1\n",MAP2ADDR(i));
	printk("/*************************end********************************/\n");
}





