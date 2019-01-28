#include<memory.h>
#include<string.h>

static int put_page(u32 cr3,u32 virAddr,u32 phyAddr,u32 attr);
static void do_wp_page(u32 err_virAddr);
static void do_no_page(u32 err_virAddr);
static int share_page(u32 virAddr);


//页错误处理函数，在interrupt.c中，被页错误异常调用
void page_fault(int errorcode,u32 ip,u16 seg)
{	
	u32 err_virAddr;

	asm("mov %%cr2,%0":"=g"(err_virAddr));	//错误地址

	DEBUG("pagefault:%d fault_ip:0x%x ret_ip:0x%x:0x%x\n",errorcode,err_virAddr,seg,ip);
	if( err_virAddr<current->start_code || \
			err_virAddr>=current->start_stack )
		panic("over flow!(page fault)");
	

	if(errorcode & 0x1)
		do_wp_page(err_virAddr);
	else
		do_no_page(err_virAddr);
}

///写页面验证。
//由于内核对用户页面写时，页面保护不会触发，所以需要提前验证并解决
static void write_verify(u32 virAddr)
{
	u32 *pde,*pte,tmp;
	pde = (u32 *)(current->cr3 +VM_START);

	if( (tmp=pde[virAddr>>22])&0x1 ){//存在对应页表
		pte=(u32 *)((tmp&0xfffff000) + VM_START);
		if( (tmp=pte[(virAddr>>12)&0x3ff])&0x1 ){//已映射物理页(非缺页)
			if(tmp & 0x2)//可写返回
				return ;
			else	//不可写保护
				do_wp_page(virAddr);
		}
	}
	//否则缺页,也可以不处理缺页，因为缺页导致中断会自动处理
	do_no_page(virAddr);
}
//// 进程空间区域写前验证函数。
// 对当前进程的地址addr 到addr+size 这一段进程空间以页为单位执行写操作前的检测操作。
// 若页面是只读的，则执行共享检验和复制页面操作（写时复制）。
void verify_area (void *addr, int size)
{
	u32 start = (u32) addr;

// 调整验证区域大小。
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
	//如果共享成功则返回,这个函数放的位置可以优化，如果页面不属于磁盘内容则无需尝试共享
	if(share_page(err_virAddr))
		return ;
	
	if(!(newPage=get_free_page()))
			panic("page not enough!(page fault)");
	memset((void *)(newPage+VM_START),0,PAGE_SIZE);//清空新页
	//注意填充的一页为:err_virAddr ~ err_virAddr +PAGE_SIZE
	if(put_page(current->cr3,err_virAddr,newPage,P_ATTR_USER_RDWR)<0)//填充页表
			panic("put_page error!(page fault)");

	//不属于磁盘内容
	if( (err_virAddr&0xfffff000) >= (current->end_data) )
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

	while(i-->0)
		*(char *)(--newPage+VM_START)=0;
	//en_page(current->cr3);//无需刷新页表缓冲，因为缺页无缓存

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
		mem_map[ADDR2MAP(oldPage)]--;
	}
	else{	//未共享，则取消写保护
		pte[(err_virAddr>>12)&0x3ff] = oldPage | 0x2;
	}
	DEBUG("viraddr:0x%x oldPage:0x%x newPage:0x%x\n",\
			err_virAddr,oldPage,pte[(err_virAddr>>12)&0x3ff]);
	en_page(current->cr3);
}


//共享页面。在缺页处理时看看能否共享页面
//必须是相同执行文件
// 返回1 - 成功，0 - 失败。。
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

	/*与task[i]是相同的执行文件,如果存在干净的对应页面，则与其共享*/
	pde=(u32 *)(task[i]->cr3+VM_START);
	if( !((tmp=pde[virAddr>>22])&0x1) )//不存在对应页目录表
		return 0;	
	pte=(u32 *)((tmp&0xfffff000) + VM_START);
	if( !((tmp=pte[(virAddr>>12)&0x3ff])&0x1) )//缺页
		return 0;	
	if(tmp&0x40)//如果页面脏(已修改)
		return 0;

	
	//可共享,task[i]页表对应目录项修改为只读
	pte[(virAddr>>12)&0x3ff]=tmp & ~0x2;//只读共享
			
	//使当前进程共享此只读页
	put_page(current->cr3,virAddr,tmp,P_ATTR_USER_RDONLY);
	mem_map[ADDR2MAP(tmp)]++;//页面引用次数+1
			
	//en_page(current->cr3);//缺页不会有页表缓存，所以无需刷新
	return 1;

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
		//put_page(cr3,virAddr,phyAddr,P_ATTR_KERNEL_RD);
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
		pde[virAddr>>22]=tmp | attr | 0x2;//页目录表不限制读写权限
		//pde[virAddr>>22]=tmp | attr ;//页目录表不限制读写权限
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
		
		DEBUG("copy_page:0x:%x\n",i<<22);
		pte_cur=(u32 *)((pde_cur[i] & 0xfffff000)+VM_START);
		//页目录指向新建立的页表，复制页表内容
		if(!(tmp=get_free_page()))
			return -1;
		pde_new[i]=tmp | P_ATTR_USER_RDWR;
		pte_new=(u32 *)(tmp+VM_START);
		memset(pte_new,0,PAGE_SIZE);

		for(n=0;n<1024;n++)
		{
			pte_cur[n] &= ~0x2;	//只读权限
			pte_new[n] = pte_cur[n];
			mem_map[ADDR2MAP(pte_cur[n])]++;//低12位对页号无影响
		}
	}	
	/*这里必须更新缓冲区，这个bug我找了整整一天，现在很兴奋，
	如果不更新，父进程返回会使用旧的可读写的页表，
	然后，可能会修改堆栈,导致子进程返回出错*/
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
