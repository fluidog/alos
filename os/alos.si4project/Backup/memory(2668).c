#include<memory.h>

#define LOW_MEM 0x100000	// 内存低端（1MB）。
#define PAGING_MEMORY (15*1024*1024)		// 分页内存15MB。主内存区最多15M。
#define PAGING_PAGES (PAGING_MEMORY>>12)	// 分页后的物理内存页数。
#define ADDR2MAP(addr) (((addr)-LOW_MEM)>>12)	// 物理内存 -> 页号
#define MAP2ADDR(page)	(((page)<<12)+LOW_MEM)	//页号 -> 物理内存


static signed char mem_map [ PAGING_PAGES ] = {0};

u32 get_free_page (void)
{
	int i;
	for(i=0;i<PAGING_PAGES;i++)
		if(!mem_map[i])
			break;

	if(i==PAGING_PAGES){panic("get free page error!!!\n");
		return 0;}
	mem_map[i]++;
	return LOW_MEM+i*PAGE_SIZE;
}


int free_page (unsigned long addr)
{
	if (addr < LOW_MEM) 
		return -1;// 如果物理地址addr 小于内存低端（1MB），则返回。
	if (addr >= LOW_MEM+PAGING_MEMORY)// 如果物理地址addr>=内存最高端，则显示出错信息。
		return -1;

	if (mem_map[ADDR2MAP(addr)]>0){
		mem_map[ADDR2MAP(addr)]--;
		return  0;// 如果对应内存页面映射字节不等于0，则减1 返回。	
	}
	return -1;
}


void mem_init()
{
	int i;
	for (i=0 ; i<PAGING_PAGES ; i++)
		mem_map[i] = 0;
	
}

void display_mm()
{
	printf("/****************physical memory all used*******************/\n");
	int i,flag;
	for(i=0,flag=0;i<PAGING_PAGES;i++)
	{						
		if(mem_map[i] || flag){		
			if(mem_map[i] && !flag){
				printf("0x%x    ~    ",MAP2ADDR(i));
				flag=1;
			}
			if(!mem_map[i] && flag){
				printf("0x%x-1\n",MAP2ADDR(i));
				flag=0;
			}
		}
	}
	if(flag)
		printf("0x%x-1\n",MAP2ADDR(i));
	printf("/*************************end********************************/\n");
}





