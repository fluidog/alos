#include<memory.h>

#define LOW_MEM 0x100000	// �ڴ�Ͷˣ�1MB����
#define PAGING_MEMORY (15*1024*1024)		// ��ҳ�ڴ�15MB�����ڴ������15M��
#define PAGING_PAGES (PAGING_MEMORY>>12)	// ��ҳ��������ڴ�ҳ����
#define ADDR2MAP(addr) (((addr)-LOW_MEM)>>12)	// �����ڴ� -> ҳ��
#define MAP2ADDR(page)	(((page)<<12)+LOW_MEM)	//ҳ�� -> �����ڴ�


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
		return -1;// ��������ַaddr С���ڴ�Ͷˣ�1MB�����򷵻ء�
	if (addr >= LOW_MEM+PAGING_MEMORY)// ��������ַaddr>=�ڴ���߶ˣ�����ʾ������Ϣ��
		return -1;

	if (mem_map[ADDR2MAP(addr)]>0){
		mem_map[ADDR2MAP(addr)]--;
		return  0;// �����Ӧ�ڴ�ҳ��ӳ���ֽڲ�����0�����1 ���ء�	
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





