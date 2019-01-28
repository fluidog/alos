#include<memory.h>
#include<string.h>

static int put_page(u32 cr3,u32 virAddr,u32 phyAddr,u32 attr);
static void do_wp_page(u32 err_virAddr);
static void do_no_page(u32 err_virAddr);

void page_fault(int errorcode,u32 ip,u16 seg)
{	
	u32 err_virAddr;

	asm("mov %%cr2,%0":"=g"(err_virAddr));	//�����ַ

	DEBUG("pagefault:%d fault_ip:0x%x ret_ip:0x%x:0x%x\n",errorcode,err_virAddr,seg,ip);
	if(err_virAddr<current->start_code | err_virAddr>=current->start_stack)
		panic("over flow!(page fault)");
	

	if(errorcode==4)
		do_no_page(err_virAddr);	
	else
		do_wp_page(err_virAddr);
	printk("44\n");
}

static void do_no_page(u32 err_virAddr)
{printk("in\n");
	buffer_head * bh;
	int i,nr,block;
	u32 newPage;
	
	if(!(newPage=get_free_page()))
			panic("page not enough!(page fault)");
	memset((void *)(newPage+VM_START),0,PAGE_SIZE);//�����ҳ
	//ע������һҳΪ:err_virAddr ~ err_virAddr +PAGE_SIZE
	if(put_page(current->cr3,err_virAddr,newPage,P_ATTR_USER_RDWR)<0)//���ҳ��
			panic("put_page error!(page fault)");

	//�����ڴ�������
	if(err_virAddr&0xfffff000 >= current->end_data || \
			(err_virAddr&0xfffff000)+PAGE_SIZE<=current->start_code)
		return ;
	

	//Ŀǰ��bin��ʽ�ļ�
	block =(err_virAddr-current->start_code)/BLOCK_SIZE;
	for(i=0;i<4;i++){
		if ((nr = bmap(current->executable,block++))) {
			if (!(bh=bread(current->executable->i_dev,nr)))
				panic("buffer error");
		} else	//�ļ��ն�
			bh = NULL;
		
		if (bh) {
			memcpy((void *)(newPage+VM_START),(void *)bh->b_data,BLOCK_SIZE);
			brelse(bh);
		} else {
			memset((void *)(newPage+VM_START),0,BLOCK_SIZE);
		}
		newPage+=BLOCK_SIZE;
	}
	//�����ڴ��̵���������,��Ϊ���뿪ʼ֮ǰ�ĵ�ַ�����壬���Բ�����
	i = (err_virAddr&0xfffff000)+PAGE_SIZE - current->end_data;
	printk("i:%d newpaage:0x%x\n",i,newPage);

	while(i--)
		*(char *)(--newPage+VM_START)=0;
	//en_page(current->cr3);//����ˢ��ҳ���壬��Ϊȱҳ�޻���
	printk("22\n");
}
static void do_wp_page(u32 err_virAddr)
{
	u32 *pde,*pte,oldPage,newPage;
	//�Ժ�����������дʱ�˳�
	pde=(u32 *)(current->cr3+VM_START);
	pte=(u32 *)((pde[err_virAddr>>22] & 0xfffff000)+VM_START); //ҳ���׵�ַ

	oldPage=pte[(err_virAddr>>12)&0x3ff];//����ҳ�����Ա�־

	if(mem_map[ADDR2MAP(oldPage)]>1){//ҳ�湲��
		if(!(newPage=get_free_page()))
			panic("no memory");
		pte[(err_virAddr>>12)&0x3ff]=newPage | P_ATTR_USER_RDWR;//�ɶ�дҳ
		//����ҳ������
		memcpy((void *)((newPage&0xfffff000)+VM_START),\
			(void *)((oldPage&0xfffff000)+VM_START),PAGE_SIZE);
	}
	else{	//δ������ȡ��д����
		pte[(err_virAddr>>12)&0x3ff] = oldPage | 2;
	}	
	en_page(current->cr3);
}

//��ʼ���ں�ҳ��Ŀǰֻӳ��VM_START ~ VM_START+16MB -- 0 ~ 16MB���ں˿ռ�
void init_page()
{
	u32 cr3,* pdeHander,virAddr,phyAddr;
	
	if(!(cr3=get_free_page()))
		panic("no memeory when init kernrl page table");
	pdeHander=(u32 *)(cr3+VM_START);
	memset(pdeHander,0,PAGE_SIZE);
		
	/* V:0xc0000000~0xc1000000 -- P:0x0~0x1000000  attr:kernel*/
	for(virAddr=VM_START,phyAddr=0; virAddr<VM_START+16*1024*1024; ){
		put_page(cr3,virAddr,phyAddr,P_ATTR_KERNEL_RDWR);
		virAddr+=PAGE_SIZE; //4K
		phyAddr+=PAGE_SIZE;
	}
	
	en_page(cr3); //ˢ��ҳ��
}

/*	���ҳ���ҳĿ¼����Ӧvirtual addr��physical addr(4K)					
	��Ϊ�ں������ַ��ʼ��Ӧ�����ַ0���������������ҳ�����ں˵�����ռ�Ϊphy+VM_START
	�ڴ治�㵼��ʧ�ܷ���-1��ע�⣺���ְ취ֻ�ܷ������1GB�����ڴ�*/
static int put_page(u32 cr3,u32 virAddr,u32 phyAddr,u32 attr)
{	
 	u32 *pde,*pte,tmp;
	
	pde=(u32*)(cr3+VM_START);//ҳĿ¼���׵�ַ
	if(!(pde[virAddr>>22] & 0x1)){//�����ڶ�ӦҳĿ¼��
		if(!(tmp=get_free_page()))
			return -1;
		pde[virAddr>>22]=tmp | attr & ~0x2;//ҳĿ¼�����ƶ�дȨ��
		memset((void *)(tmp+VM_START),0,PAGE_SIZE);
	}

	pte=(u32 *)((pde[virAddr>>22] & 0xfffff000)+VM_START); //ҳ���׵�ַ
	//ҳ���Ѿ�ӳ��,����
	if(pte[(virAddr>>12)&0x3ff] & 0x1)	
		panic("page have maped");
		
	pte[(virAddr>>12)&0x3ff]=phyAddr&0xfffff000 | attr;
	return 0;
}



//ʹnewTask->cr3ָ���µ�ҳ�������Ƶ�ǰ���̵�ַ�ռ�
int copy_page_table(task_struct *newTask)
{
	u32 *pde_new,*pde_cur,*pte_cur,*pte_new,tmp,startAddr;
	int i,n;
	if(!newTask)
		return -1;			
	if(!(tmp=get_free_page()))
		return -1;
	newTask->cr3=tmp;	//�½��̻��ҳ��
	pde_new=(u32 *)(tmp+VM_START);	//��ҳĿ¼���׵�ַ
	memset(pde_new,0,PAGE_SIZE);
	pde_cur=(u32 *)(current->cr3+VM_START);	//��ǰ����ҳĿ¼���׵�ַ
	
	/*			�ں˿ռ�:0xc0000000~0xffffffff+1
	����pde���еĶ�Ӧ�ı����֮����ָ���ں˵�pte���Ӷ������ں˿ռ�	*/
	for(i=VM_START>>22;i<1024;i++)
		pde_new[i]=pde_cur[i];
	
	/*			�û��ռ�:0~0xc0000000(VM_START)				
	����ǰ���̵�pte���е�Ȩ�޸�Ϊֻ�����½��̽����µ�pte���Ʊ��������ҳ�����ô���
	ע��:�빲���ں�ҳ�治ͬ�������û��ռ���Ҫ�����µ�pte����ԭ����	*/
	for(i=0;i<VM_START>>22;i++){
		if(!(pde_cur[i] & 1))//ҳĿ¼�������
			continue ;	
		pte_cur=(u32 *)((pde_cur[i] & 0xfffff000)+VM_START);

		//ҳĿ¼ָ���½�����ҳ������ҳ������
		if(!(tmp=get_free_page()))
			return -1;
		pde_new[i]=tmp;
		pte_new=(u32 *)(tmp+VM_START);
		memset(pte_new,0,PAGE_SIZE);

		for(n=0;n<1024;n++)
		{
			pte_cur[n] &= ~2;	//ֻ��Ȩ��
			pde_new[n] = pte_cur[n];
			mem_map[ADDR2MAP(pte_cur[n])]++;//��12λ��ҳ����Ӱ��
		}
	}	
	return 0;
}

void display_page_table(u32 cr3)
{
	u32 *pdeHander=(u32 *)(cr3+VM_START);

	int pde_i,pte_j;
	for(pde_i=0;pde_i<1024;pde_i++){
		if(pdeHander[pde_i]&0x1){
			printk("V:0x%x~0x%x\n",pde_i<<22,(pde_i<<22)+4*1024*1024);
		}

	}
}
