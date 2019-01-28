#include<memory.h>
#include<string.h>

static int put_page(u32 cr3,u32 virAddr,u32 phyAddr,u32 attr);
static void do_wp_page(u32 err_virAddr);
static void do_no_page(u32 err_virAddr);

void page_fault(int errorcode,u32 ip,u16 seg)
{	
	u32 err_virAddr;

	asm("mov %%cr2,%0":"=g"(err_virAddr));	//错误地址

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
	memset((void *)(newPage+VM_START),0,PAGE_SIZE);//清空新页
	//注意填充的一页为:err_virAddr ~ err_virAddr +PAGE_SIZE
	if(put_page(current->cr3,err_virAddr,newPage,P_ATTR_USER_RDWR)<0)//填充页表
			panic("put_page error!(page fault)");

	//不属于磁盘内容
	if(err_virAddr&0xfffff000 >= current->end_data || \
			(err_virAddr&0xfffff000)+PAGE_SIZE<=current->start_code)
		return ;
	

	//目前是bin格式文件
	block =(err_virAddr-current->start_code)/BLOCK_SIZE;
	for(i=0;i<4;i++){
		if ((nr = bmap(current->executable,block++))) {
			if (!(bh=bread(current->executable->i_dev,nr)))
				panic("buffer error");
		} else	//文件空洞
			bh = NULL;
		
		if (bh) {
			memcpy((void *)(newPage+VM_START),(void *)bh->b_data,BLOCK_SIZE);
			brelse(bh);
		} else {
			memset((void *)(newPage+VM_START),0,BLOCK_SIZE);
		}
		newPage+=BLOCK_SIZE;
	}
	//不属于磁盘的内容清零,因为代码开始之前的地址无意义，所以不清理
	i = (err_virAddr&0xfffff000)+PAGE_SIZE - current->end_data;
	printk("i:%d newpaage:0x%x\n",i,newPage);

	while(i--)
		*(char *)(--newPage+VM_START)=0;
	//en_page(current->cr3);//无需刷新页表缓冲，因为缺页无缓存
	printk("22\n");
}
static void do_wp_page(u32 err_virAddr)
{
	u32 *pde,*pte,oldPage,newPage;
	//以后加入往代码段写时退出
	pde=(u32 *)(current->cr3+VM_START);
	pte=(u32 *)((pde[err_virAddr>>22] & 0xfffff000)+VM_START); //页表首地址

	oldPage=pte[(err_virAddr>>12)&0x3ff];//含有页面属性标志

	if(mem_map[ADDR2MAP(oldPage)]>1){//页面共享
		if(!(newPage=get_free_page()))
			panic("no memory");
		pte[(err_virAddr>>12)&0x3ff]=newPage | P_ATTR_USER_RDWR;//可读写页
		//复制页面内容
		memcpy((void *)((newPage&0xfffff000)+VM_START),\
			(void *)((oldPage&0xfffff000)+VM_START),PAGE_SIZE);
	}
	else{	//未共享，则取消写保护
		pte[(err_virAddr>>12)&0x3ff] = oldPage | 2;
	}	
	en_page(current->cr3);
}

//初始化内核页表，目前只映射VM_START ~ VM_START+16MB -- 0 ~ 16MB的内核空间
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
	
	en_page(cr3); //刷新页表
}

/*	填充页表和页目录，对应virtual addr和physical addr(4K)					
	因为内核虚拟地址起始对应物理地址0处，所以新申请的页面在内核的虚拟空间为phy+VM_START
	内存不足导致失败返回-1，注意：这种办法只能访问最多1GB物理内存*/
static int put_page(u32 cr3,u32 virAddr,u32 phyAddr,u32 attr)
{	
 	u32 *pde,*pte,tmp;
	
	pde=(u32*)(cr3+VM_START);//页目录表首地址
	if(!(pde[virAddr>>22] & 0x1)){//不存在对应页目录项
		if(!(tmp=get_free_page()))
			return -1;
		pde[virAddr>>22]=tmp | attr & ~0x2;//页目录表不限制读写权限
		memset((void *)(tmp+VM_START),0,PAGE_SIZE);
	}

	pte=(u32 *)((pde[virAddr>>22] & 0xfffff000)+VM_START); //页表首地址
	//页面已经映射,出错
	if(pte[(virAddr>>12)&0x3ff] & 0x1)	
		panic("page have maped");
		
	pte[(virAddr>>12)&0x3ff]=phyAddr&0xfffff000 | attr;
	return 0;
}



//使newTask->cr3指向新的页表，并复制当前进程地址空间
int copy_page_table(task_struct *newTask)
{
	u32 *pde_new,*pde_cur,*pte_cur,*pte_new,tmp,startAddr;
	int i,n;
	if(!newTask)
		return -1;			
	if(!(tmp=get_free_page()))
		return -1;
	newTask->cr3=tmp;	//新进程获得页表
	pde_new=(u32 *)(tmp+VM_START);	//新页目录表首地址
	memset(pde_new,0,PAGE_SIZE);
	pde_cur=(u32 *)(current->cr3+VM_START);	//当前进程页目录表首地址
	
	/*			内核空间:0xc0000000~0xffffffff+1
	复制pde表中的对应的表项，与之共享指向内核的pte表，从而共享内核空间	*/
	for(i=VM_START>>22;i<1024;i++)
		pde_new[i]=pde_cur[i];
	
	/*			用户空间:0~0xc0000000(VM_START)				
	将当前进程的pte表中的权限改为只读，新进程建立新的pte表复制表项，并增加页面引用次数
	注意:与共享内核页面不同，共享用户空间需要建立新的pte复制原内容	*/
	for(i=0;i<VM_START>>22;i++){
		if(!(pde_cur[i] & 1))//页目录表项不存在
			continue ;	
		pte_cur=(u32 *)((pde_cur[i] & 0xfffff000)+VM_START);

		//页目录指向新建立的页表，复制页表内容
		if(!(tmp=get_free_page()))
			return -1;
		pde_new[i]=tmp;
		pte_new=(u32 *)(tmp+VM_START);
		memset(pte_new,0,PAGE_SIZE);

		for(n=0;n<1024;n++)
		{
			pte_cur[n] &= ~2;	//只读权限
			pde_new[n] = pte_cur[n];
			mem_map[ADDR2MAP(pte_cur[n])]++;//低12位对页号无影响
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
