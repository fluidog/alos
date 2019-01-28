asm("jmp start_kernel");
#include<linux/config.h>
#include<memory.h>
#include<string.h>
#include<linux/kernel.h>
#include<malloc.h>
#include<asm/interrupt.h>
#include<linux/tty.h>
#include<fat.h>
#include<linux/sched.h>

#include<unistd.h>
#include<linux/sys.h>

#include<linux/fs.h>

#include<fcntl.h>
#include<drivers/ide.h>
void all_init(void)
{
	tty_init();				//终端
	init_vmalloc_area();	//系统堆		0xc0080000~0xc0090000
	
	init_gdt();				//段表
	mem_init();				//内存
	init_fat16();		//文件系统	
	
	init_page();		//页表
	sched_init();		//任务     //必须在页表初始化之前
		
	interrupt_init();		//中断
	buffer_init(1*1024*1024+VM_START);	//缓冲区
}




void prb(unsigned char *addr)
{
	int x,y;
	
	for(x=0;x<16;x++)
		printk("%02x ",*addr++);
	printk("\n");

}

void start_kernel()
{
	all_init();
	
	ROOT_DEV=IDE_DRIVER;
	
	asm("mov $0x10,%%ax\n"
		"mov %%ax,%%fs"
		:::"eax");
	
	mount_root();
	printk("create ok\n");
	m_inode *inode;
	inode=get_dir("/a.c");
	printk("inum:%d dev:0x%x\n size:%d block:%d\n",\
			inode->i_num,inode->i_dev,inode->i_size,inode->i_blocks);
	ls(inode);
	while(1);
	int fd;
	if((fd=sys_open("/a.c",O_RDONLY,0777))<0)
		panic("open file");
	printk("fd:%d\n");
	sys_sync();
	
	while(1);
	char buf[100];
	int x=sys_read(fd,buf,100);
	printk("x=%d\n",x);
	printk("%s\n",buf);
	
	while(1);
	
	asm("push %0\n"
		"push $0x500\n"
		"push %1\n"
		"push %2\n"
		"mov %0,%%eax\n"
		"mov %%ax,%%ds\n"
		"mov %%ax,%%es\n"
		"lret"
		::"i"(SELECTOR(USER_DATA,PL_USER)),
		"i"(SELECTOR(USER_CODE,PL_USER)),
		"g"(0));
}

void ls(m_inode *dir)
{
	ext2_dir_entry_2 *entry;
	int i,j,block;
	buffer_head *bh;
	for(i=0;(i+1)*BLOCK_SIZE<=dir->i_size;i++){
		if(!(block=bmap(dir,i)))
			continue;
		if(!(bh=bread(dir->i_dev,block)))
			panic("ls");
		entry=(ext2_dir_entry_2*)bh->b_data;
		while(1){
			if(!entry->rec_len || (u32)entry >= (u32)bh->b_data+BLOCK_SIZE )
				break;
			for(j=0;j<entry->name_len;j++)
				printk("%c",entry->name[j]);
			printk("(%d)  ",entry->inode);
			entry=(ext2_dir_entry_2 *)((u32)entry + entry->rec_len);
		}
		brelse(bh);
	}
	printk("\n");
}
void lsi(int dev,int nr)
{
	m_inode *inode;
	if(!(inode=iget(dev,nr)))
		panic("no inode");
	printk("(%d)size:%d izone[0]:0x%x\n",\
		inode->i_mode,inode->i_size,inode->i_zone[0]);
}






