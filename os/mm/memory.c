#include<memory.h>
static int page_num;	//���ڼ�¼�ϴη���ĵ�ַ�����Դ���ʿ����ҵ�����ҳ
u32  get_free_page (void)
{
	int back=page_num;
	do{
		page_num=(page_num+1)%PAGING_PAGES;
		if(mem_map[page_num])
			continue;

		//�ҵ�����ҳ
		mem_map[page_num]++;
		return MAP2ADDR(page_num);

	}while(page_num!=back);	
	
	return 0;//��Ϊ0��ַ�����ڿɷ����ڴ棬�������ڴ����־
}
 

void free_page (u32 phyAddr)
{
	if (phyAddr < LOW_MEM || phyAddr >= HIGH_MEMORY)	//ϵͳ�ں˴���
		panic("try to free kernel or nonexistent page");
	
	if (mem_map[ADDR2MAP(phyAddr)]>0)
		mem_map[ADDR2MAP(phyAddr)]--;	
	
	panic("trying to free free page");
}



void mem_init()
{
	int i,count;
	page_num=0;
	if(HIGH_MEMORY & 0xfff)
			panic("memory can`t div exactly by PAGE_SIZE");
	count=(HIGH_MEMORY-LOW_MEM)/PAGE_SIZE;
	for (i=0 ; i<PAGING_PAGES ; i++)
		mem_map[i] = 1;// ����ҳ��ӳ������ȫ�ó�USED��	
	i=0;
	while (count-->0)// �����Щ����ҳ���Ӧ��ҳ��ӳ���������㡣
		mem_map[i++]=0;
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





