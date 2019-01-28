#include<memory.h>
#include<string.h>
#include<fat.h>
#include<linux/config.h>



static int put_page(pde_t* pdeHander,u32 virAddr,u32 phyAddr,u32 attr);
static int page_load_file(m_inode *execfile, char *buf,off_t off,int count);

void page_fault(int errorcode,u32 ip,u16 seg)
{	
	u32 err_virAddr,start_virAddr,end_virAddr;
	u32 new_phyAddr;
	off_t off;
	int ret,count;
	pde_t * pdeHander=(pde_t *)(current->cr3+VM_START);
	asm("mov %%cr2,%0":"=g"(err_virAddr));	//错误地址

	DEBUG("err:%d fault_ip:0x%x ret_ip: 0x%x:0x%x\n",errorcode,err_virAddr,seg,ip);
	if(err_virAddr<current->start_code | err_virAddr>=current->start_stack)
		panic("over flow!(page fault)");

	start_virAddr=err_virAddr & (~(PAGE_SIZE-1));//对齐错误地址
	end_virAddr=start_virAddr+PAGE_SIZE;
	
	if(!(new_phyAddr=get_free_page()))
		panic("page not enough!(page fault)");
	memset((void *)(new_phyAddr+VM_START),0,PAGE_SIZE);//清空新页
	//注意填充的一页为:err_virAddr ~ err_virAddr +PAGE_SIZE
	if(put_page(pdeHander,start_virAddr,new_phyAddr,USER_P_ATTR)<0)//填充页表
		panic("put_page error!(page fault)");
	//不属于磁盘内容
	if(start_virAddr>=current->end_data || \
			end_virAddr<=current->start_code){
		printk("start:0x%x end:0x%x\n",start_virAddr,current->end_data);
		return ;
			}
	
	//复制物理磁盘程序到内存
	off=err_virAddr-current->start_code;//文件偏移
	if(off<0){
		start_virAddr=current->start_code;
		off=0;
	}
	if(end_virAddr>current->end_data)
		end_virAddr=current->end_data;
	
	count=end_virAddr-start_virAddr;
	if(count!=page_load_file(current->executable,(char *)start_virAddr,off,count))
		printk("exec file read have some troble\n");
}


//page fault 独用读文件函数
static int page_load_file(m_inode *execfile, char *buf,off_t off,int count)
{
	int left,chars,nr;
	buffer_head * bh;
	char * p;
	if(!execfile || !buf || off<0)
		return -1;	
	
	//读不能超出文件
	if (count+off > execfile->i_size)
		count = execfile->i_size - off;
	if ((left=count)<=0)
		return 0;
		
	while (left) {
		if ((nr = bmap(execfile,(off)/BLOCK_SIZE))) {
			if (!(bh=bread(execfile->i_dev,nr)))
				panic("buffer error");
		} else
			bh = NULL;
			
		nr = off % BLOCK_SIZE;
		chars=BLOCK_SIZE-nr<left?BLOCK_SIZE-nr:left;
		off += chars;//文件定位符
		left -= chars;//剩余字节
		if (bh) {
			p = nr + bh->b_data;
			while (chars-->0)
				*buf++=*p++;
			brelse(bh);
		} else {
			while (chars-->0)	//文件空洞
				*buf++=0;
		}
	}
	return count-left;
}


void init_page()
{
	pde_t * pdeHander;
	pdeHander=(pde_t *)(get_free_page()+VM_START);
	
	memset(pdeHander,0,PAGE_SIZE);
	
	u32 virAddr,phyAddr;
	/* V:0xc0000000~0xc1000000 -- P:0x0~0x1000000  attr:kernel*/
	for(virAddr=VM_START,phyAddr=0; virAddr<VM_START+16*1024*1024; ){
		put_page(pdeHander,virAddr,phyAddr,KERNEL_P_ATTR);
		virAddr+=PAGE_SIZE; //4K
		phyAddr+=PAGE_SIZE;
	}

	EN_PAGE((u32)pdeHander-VM_START); //物理地址
}

/* 填充页表和页目录，对应virtual addr和physical addr(4K)*/
static int put_page(pde_t* pdeHander,u32 virAddr,u32 phyAddr,u32 attr)
{	
	pte_t *pteHander;
	if(pdeHander[virAddr>>22] & 0x1){		//存在对应页表			
		pteHander=(pte_t *)((u32)(pdeHander[virAddr>>22]&0xfffff000)+VM_START);//虚拟地址(线性地址)
	}else{	//申请一页内存作为页表
		if((pteHander=(pte_t*)(get_free_page()+VM_START))==(pte_t*)VM_START)
			return -1;
		memset(pteHander,0,PAGE_SIZE);
		pdeHander[virAddr>>22]=(pde_t)( ((u32)pteHander)-VM_START | attr );//物理地址
	}


	if(pteHander[(virAddr>>12)&0x3ff] & 0x1){		//已经映射
		if(free_page(pteHander[(virAddr>>12)&0x3ff] & 0xfffff000)<0)
			return -1;
	}
	
	pteHander[(virAddr>>12)&0x3ff]=(pte_t)(phyAddr&0xfffff000 | attr);
	
	return 0;
}

//使新进程获得新的页表，页表映射内核16MB地址空间，并使newTask->cr3指向新的页表
int copy_kernel_page_table(task_struct *newTask)
{
	if(!newTask)
		return -1;
		
	pde_t *pdeHander_new,*pdeHander_old;
	newTask->cr3=get_free_page();
	if((pdeHander_new=(pde_t *)(newTask->cr3+VM_START))==(pde_t *)VM_START)
		return -1;
	memset(pdeHander_new,0,PAGE_SIZE);
	pdeHander_old=(pde_t *)(current->cr3+VM_START);

	u32 startAddr=VM_START;
	
	/*0xc0000000~0xc1000000 -- 0x0~0x1000000 (16MB)
	复制pde表中的对应的表项，与之共享指向内核的pte表，从而共享内核空间*/
	for(startAddr=VM_START;startAddr<VM_START+16*1024*1024;startAddr+=4*1024*1024)
		pdeHander_new[startAddr>>22]=pdeHander_old[startAddr>>22];
	return 0;
}


/*	将当前进程用户地址空间(startAddr~startAddr+size)的内容，复制到新进程相同的虚拟地址处 
	注意:如果start或end未对齐,会复制包含此地址的一整页内容 */
int copy_page_table(task_struct *newTask,u32 startAddr,u32 size)
{
	if(!newTask | startAddr>=VM_START | startAddr+size>VM_START)
		return -1;
	
	pde_t *pdeHander;
	if((pdeHander=(pde_t *)(newTask->cr3+VM_START))==(pde_t *)VM_START)
		return -1;

	u32 endAddr=startAddr+size;
	startAddr=startAddr/PAGE_SIZE*PAGE_SIZE;

	for(;startAddr<endAddr;startAddr+=PAGE_SIZE){
		u32 phyAddr;
		if((phyAddr=get_free_page())==0)
			return -1;
		memcpy((void *)(phyAddr+VM_START),(void *)startAddr,PAGE_SIZE);//复制页内容
		if(put_page(pdeHander,startAddr,phyAddr,USER_P_ATTR)<0)
			return -1;
	}
}


void display_page_table(u32 cr3)
{
	pde_t *pdeHander=(pde_t *)(cr3+VM_START);

	int pde_i,pte_j;
	for(pde_i=0;pde_i<1024;pde_i++){
		if(pdeHander[pde_i]&0x1){
			printk("V:0x%x~0x%x\n",pde_i<<22,(pde_i<<22)+4*1024*1024);
		}

	}
}
