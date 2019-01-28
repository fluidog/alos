#include<memory.h>
#include<string.h>

static int put_page(u32 cr3,u32 virAddr,u32 phyAddr,u32 attr);
static void do_wp_page(u32 err_virAddr);
static void do_no_page(u32 err_virAddr);
static int share_page(u32 virAddr);


//ҳ������������interrupt.c�У���ҳ�����쳣����
void page_fault(int errorcode,u32 ip,u16 seg)
{	
	u32 err_virAddr;

	asm("mov %%cr2,%0":"=g"(err_virAddr));	//�����ַ

	DEBUG("pagefault:%d fault_ip:0x%x ret_ip:0x%x:0x%x\n",errorcode,err_virAddr,seg,ip);
	if( err_virAddr<current->start_code || \
			err_virAddr>=current->start_stack )
		panic("over flow!(page fault)");
	

	if(errorcode & 0x1)
		do_wp_page(err_virAddr);
	else
		do_no_page(err_virAddr);
}

///дҳ����֤��
//�����ں˶��û�ҳ��дʱ��ҳ�汣�����ᴥ����������Ҫ��ǰ��֤�����
static void write_verify(u32 virAddr)
{
	u32 *pde,*pte,tmp;
	pde = (u32 *)(current->cr3 +VM_START);

	if( (tmp=pde[virAddr>>22])&0x1 ){//���ڶ�Ӧҳ��
		pte=(u32 *)((tmp&0xfffff000) + VM_START);
		if( (tmp=pte[(virAddr>>12)&0x3ff])&0x1 ){//��ӳ������ҳ(��ȱҳ)
			if(tmp & 0x2)//��д����
				return ;
			else	//����д����
				do_wp_page(virAddr);
		}
	}
	//����ȱҳ,Ҳ���Բ�����ȱҳ����Ϊȱҳ�����жϻ��Զ�����
	do_no_page(virAddr);
}
//// ���̿ռ�����дǰ��֤������
// �Ե�ǰ���̵ĵ�ַaddr ��addr+size ��һ�ν��̿ռ���ҳΪ��λִ��д����ǰ�ļ�������
// ��ҳ����ֻ���ģ���ִ�й������͸���ҳ�������дʱ���ƣ���
void verify_area (void *addr, int size)
{
	u32 start = (u32) addr;

// ������֤�����С��
	size += start & 0xfff;

	while (size > 0)
	{
		size -= 4096;
		write_verify (start);
		start += 4096;
	}
}

static void do_no_page(u32 err_virAddr)
{
	buffer_head * bh;
	int i,nr,block;
	u32 newPage;
	//�������ɹ��򷵻�,��������ŵ�λ�ÿ����Ż������ҳ�治���ڴ������������賢�Թ���
	if(share_page(err_virAddr))
		return ;
	
	if(!(newPage=get_free_page()))
			panic("page not enough!(page fault)");
	memset((void *)(newPage+VM_START),0,PAGE_SIZE);//�����ҳ
	//ע������һҳΪ:err_virAddr ~ err_virAddr +PAGE_SIZE
	if(put_page(current->cr3,err_virAddr,newPage,P_ATTR_USER_RDWR)<0)//���ҳ��
			panic("put_page error!(page fault)");

	//�����ڴ�������
	if( (err_virAddr&0xfffff000) >= (current->end_data) )
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

	while(i-->0)
		*(char *)(--newPage+VM_START)=0;
	//en_page(current->cr3);//����ˢ��ҳ���壬��Ϊȱҳ�޻���

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
		mem_map[ADDR2MAP(oldPage)]--;
	}
	else{	//δ������ȡ��д����
		pte[(err_virAddr>>12)&0x3ff] = oldPage | 0x2;
	}
	DEBUG("viraddr:0x%x oldPage:0x%x newPage:0x%x\n",\
			err_virAddr,oldPage,pte[(err_virAddr>>12)&0x3ff]);
	en_page(current->cr3);
}


//����ҳ�档��ȱҳ����ʱ�����ܷ���ҳ��
//��������ִͬ���ļ�
// ����1 - �ɹ���0 - ʧ�ܡ���
static int share_page(u32 virAddr)
{
	int i;
	u32 *pde,*pte,tmp;
	if(current->executable->i_count <=1 )
		return 0;
	for(i=0;i<NR_TASKS;i++)
		if(task[i] && task[i]->executable == current->executable)
			if(task[i] != current)
				break;
	if(i==NR_TASKS)
		return 0;

	/*��task[i]����ͬ��ִ���ļ�,������ڸɾ��Ķ�Ӧҳ�棬�����乲��*/
	pde=(u32 *)(task[i]->cr3+VM_START);
	if( !((tmp=pde[virAddr>>22])&0x1) )//�����ڶ�ӦҳĿ¼��
		return 0;	
	pte=(u32 *)((tmp&0xfffff000) + VM_START);
	if( !((tmp=pte[(virAddr>>12)&0x3ff])&0x1) )//ȱҳ
		return 0;	
	if(tmp&0x40)//���ҳ����(���޸�)
		return 0;

	
	//�ɹ���,task[i]ҳ���ӦĿ¼���޸�Ϊֻ��
	pte[(virAddr>>12)&0x3ff]=tmp & ~0x2;//ֻ������
			
	//ʹ��ǰ���̹����ֻ��ҳ
	put_page(current->cr3,virAddr,tmp,P_ATTR_USER_RDONLY);
	mem_map[ADDR2MAP(tmp)]++;//ҳ�����ô���+1
			
	//en_page(current->cr3);//ȱҳ������ҳ���棬��������ˢ��
	return 1;

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
		//put_page(cr3,virAddr,phyAddr,P_ATTR_KERNEL_RD);
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
		pde[virAddr>>22]=tmp | attr | 0x2;//ҳĿ¼�����ƶ�дȨ��
		//pde[virAddr>>22]=tmp | attr ;//ҳĿ¼�����ƶ�дȨ��
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
		
		DEBUG("copy_page:0x:%x\n",i<<22);
		pte_cur=(u32 *)((pde_cur[i] & 0xfffff000)+VM_START);
		//ҳĿ¼ָ���½�����ҳ������ҳ������
		if(!(tmp=get_free_page()))
			return -1;
		pde_new[i]=tmp | P_ATTR_USER_RDWR;
		pte_new=(u32 *)(tmp+VM_START);
		memset(pte_new,0,PAGE_SIZE);

		for(n=0;n<1024;n++)
		{
			pte_cur[n] &= ~0x2;	//ֻ��Ȩ��
			pte_new[n] = pte_cur[n];
			mem_map[ADDR2MAP(pte_cur[n])]++;//��12λ��ҳ����Ӱ��
		}
	}	
	/*���������»����������bug����������һ�죬���ں��˷ܣ�
	��������£������̷��ػ�ʹ�þɵĿɶ�д��ҳ��
	Ȼ�󣬿��ܻ��޸Ķ�ջ,�����ӽ��̷��س���*/
	en_page(current->cr3);
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
