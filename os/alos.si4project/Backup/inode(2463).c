/*
 *  linux/fs/inode.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <string.h> 

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/system.h>

m_inode inode_table[NR_INODE];

static void read_inode(struct m_inode * inode);
//static void write_inode(struct m_inode * inode);

static inline void wait_on_inode(struct m_inode * inode)
{
	cli();
	while (inode->i_lock)
		sleep_on(&inode->i_wait);
	sti();
}

static inline void lock_inode(struct m_inode * inode)
{
	cli();
	while (inode->i_lock)
		sleep_on(&inode->i_wait);
	inode->i_lock=1;
	sti();
}

static inline void unlock_inode(struct m_inode * inode)
{
	inode->i_lock=0;
	wake_up(&inode->i_wait);
}

void invalidate_inodes(int dev)
{
	int i;
	struct m_inode * inode;

	inode = 0+inode_table;
	for(i=0 ; i<NR_INODE ; i++,inode++) {
		wait_on_inode(inode);
		if (inode->i_dev == dev) {
			if (inode->i_count)
				printk("inode in use on removed disk\n\r");
			inode->i_dev = inode->i_dirt = 0;
		}
	}
}
static void read_inode(m_inode * inode)
{
	m_super_block * sb;
	buffer_head * bh;
	int block,group;
	group_desc *gd;
	if(!inode->i_num ||!inode)
		return;
	
	lock_inode(inode);
	if (!(sb=get_super(inode->i_dev)))
		panic("trying to read inode without dev");
	
	//结点所在组号
	group=(inode->i_num-1) / sb->s_inodes_per_group;
	if(!(bh=get_group(sb,group,&gd)))
		panic("unable to read group");
	
	//结点所在物理块号
	block=gd->bg_inode_table+(inode->i_num-1) % sb->s_inodes_per_group /INODES_PER_BLOCK;
	brelse(bh);
	
	if (!(bh=bread(inode->i_dev,block)))
		panic("unable to read i-node block");
	//根据偏移读结点
	*(struct d_inode *)inode =
		((struct d_inode *)bh->b_data)
			[(inode->i_num-1)%sb->s_inodes_per_group%INODES_PER_BLOCK];
			
	brelse(bh);
	unlock_inode(inode);
}

static void write_inode(m_inode * inode)
{
	m_super_block * sb;
	buffer_head * bh;
	int block,group;
	group_desc *gd;
	if (!inode->i_dirt || !inode->i_dev || !inode) 
			return;

	lock_inode(inode);	
	if (!(sb=get_super(inode->i_dev)))
		panic("trying to write inode without device");

	group=(inode->i_num-1) / sb->s_inodes_per_group;
	
	if(!(bh=get_group(sb,group,&gd)))
		panic("unable to read group");	
	block=gd->bg_inode_table+(inode->i_num-1) % sb->s_inodes_per_group /INODES_PER_BLOCK;
	brelse(bh);
	
	if (!(bh=bread(inode->i_dev,block)))
		panic("unable to read i-node block");
	((struct d_inode *)bh->b_data)\
		[(inode->i_num-1)%sb->s_inodes_per_group%INODES_PER_BLOCK]=
			*(struct d_inode *)inode ;
		
	bh->b_dirt=1;
	inode->i_dirt=0;
	brelse(bh);
	unlock_inode(inode);
}

void sync_inodes(void)
{
	int i;
	m_inode * inode;

	inode = 0+inode_table;
	for(i=0 ; i<NR_INODE ; i++,inode++) {
		wait_on_inode(inode);
		if (inode->i_dirt && !inode->i_pipe)
			write_inode(inode);
	}
}


//取文件数据块对应的磁盘数据块
//12直接、1间接、1双间接、1三间接
#define GET(zone,index)	(*((unsigned int *)(zone)+((index)&0xff)))
static int _bmap(m_inode * inode,int block,int create)
{
	buffer_head * bh;
	int i;
	if(!inode)
		return 0;
	if (block<0||block >= 12+256+256*256+256*256*256)
		panic("_bmap: block<0 || block>max");
	//12个直接块
	if (block<12) {
		if (create && !GET(inode->i_zone,block))
			if ((GET(inode->i_zone,block)=new_block(inode->i_dev)->b_blocknr)) {
				inode->i_ctime=CURRENT_TIME;
				inode->i_dirt=1;
				inode->i_blocks+=2;//此处指物理扇区
			}
		return GET(inode->i_zone,block);	
	}

	//1个间接块
	block -= 12;
	if (block<256) {
		if (create && !GET(inode->i_zone,12)){
			if ((GET(inode->i_zone,12)=new_block(inode->i_dev)->b_blocknr)) {
				inode->i_blocks+=2;//此处指物理扇区
				inode->i_dirt=1;
				inode->i_ctime=CURRENT_TIME;
			}
		}
		if (!GET(inode->i_zone,12))
			return 0;
		if (!(bh = bread(inode->i_dev,GET(inode->i_zone,12))))
			return 0;
		i = GET(bh->b_data,block);
		if (create && !i)
			if ((i=new_block(inode->i_dev)->b_blocknr)) {
				GET(bh->b_data,block)=i;
				bh->b_dirt=1;
				inode->i_blocks+=2;
				inode->i_dirt=1;
				inode->i_ctime=CURRENT_TIME;			
			}
		brelse(bh);
		return i;
	}

	//一个双间接
	block -= 256;
	if(block<256*256){
		if (create && !GET(inode->i_zone,13))
			if ((GET(inode->i_zone,13)=new_block(inode->i_dev)->b_blocknr)) {
				inode->i_dirt=1;
				inode->i_ctime=CURRENT_TIME;
				inode->i_blocks+=2;
			}		
			//读间接
		if (!GET(inode->i_zone,13))
			return 0;
		if (!(bh=bread(inode->i_dev,GET(inode->i_zone,13))))
			return 0;
		i=GET(bh->b_data,block>>8);
		if (create && !i)
			if ((i=new_block(inode->i_dev)->b_blocknr)) {
				GET(bh->b_data,block>>8)=i;
				bh->b_dirt=1;
				inode->i_blocks+=2;
				inode->i_dirt=1;
				inode->i_ctime=CURRENT_TIME;	
			}
		brelse(bh);
			//读双间接
		if (!i)
			return 0;
		if (!(bh=bread(inode->i_dev,i)))
			return 0;
		i = GET(bh->b_data,block&0xff);
		if (create && !i)
			if ((i=new_block(inode->i_dev)->b_blocknr)) {
				GET(bh->b_data,block&0xff) = i;
				bh->b_dirt=1;
				inode->i_blocks+=2;
				inode->i_dirt=1;
				inode->i_ctime=CURRENT_TIME;
			}
		brelse(bh);
		return i;
	}
	
	//一个三间接块
	block-=256*256;
	if (create && !GET(inode->i_zone,14))
		if ((GET(inode->i_zone,14)=new_block(inode->i_dev)->b_blocknr)) {
			inode->i_dirt=1;
			inode->i_ctime=CURRENT_TIME;
			inode->i_blocks+=2;
		}
		//读间接
	if (!GET(inode->i_zone,14))
		return 0;
	if (!(bh=bread(inode->i_dev,GET(inode->i_zone,14))))
		return 0;
	i = GET(bh->b_data,block>>16);
	if (create && !i)
		if ((i=new_block(inode->i_dev)->b_blocknr)) {
			GET(bh->b_data,block>>16)=i;
			inode->i_dirt=1;
			inode->i_ctime=CURRENT_TIME;
			inode->i_blocks+=2;
			bh->b_dirt=1;
		}
	brelse(bh);
		//读双间接	
	if (!i)
		return 0;
	if (!(bh=bread(inode->i_dev,i)))
		return 0;
	i=GET(bh->b_data,(block>>8)&0xff);
	if (create && !i)
		if ((i=new_block(inode->i_dev)->b_blocknr)) {
			GET(bh->b_data,(block>>8)&0xff)=i;
			inode->i_dirt=1;
			inode->i_ctime=CURRENT_TIME;
			inode->i_blocks+=2;
			bh->b_dirt=1;
		}
	brelse(bh);
		//读三间接
	if(!i)
		return 0;
	if(!(bh=bread(inode->i_dev,i)))
		return 0;
	i=GET(bh->b_data,block&0xff);
	if(create && !i)
		if((i=new_block(inode->i_dev)->b_blocknr)){
			GET(bh->b_data,block&0xff)=i;
			inode->i_dirt=1;
			inode->i_ctime=CURRENT_TIME;
			inode->i_blocks+=2;
			bh->b_dirt=1;
		}
	brelse(bh);
	return i;
}

int bmap(m_inode * inode,int block)
{
	return _bmap(inode,block,0);
}

int create_block(m_inode * inode, int block)
{
	return _bmap(inode,block,1);
}
/* 释放文件占用的空间,但不清除文件结点的内容，
因此，调用此函数后，应使结点无效，否则，结点内容是错误的.
不清除结点内容主要用于误删恢复
备注：目前此功能不完善，没有删除二级和三级块表*/
void truncate (m_inode *inode)
{
	int i,blocks,block;
	blocks=inode->i_blocks / 2;//占用块数/blocks为物理扇区的二倍
	for(i=0;blocks;i++){
		if(!(block=bmap(inode,i)))//空洞
			continue;
		free_block(inode->i_dev,block);
		blocks--;
	}
}
	
void iput(m_inode * inode)
{
	if (!inode)
		return;
	
	if (!inode->i_count)
		panic("iput: trying to free free inode");

	wait_on_inode(inode);
repeat:
	if(inode->i_count>1){
		inode->i_count--;
		return ;
	}
	
	if(inode->i_dirt){
		write_inode(inode);	/* we can sleep - so do again */
		wait_on_inode(inode);
		goto repeat;
	}
	inode->i_count--;
	return;
}

//如果所有节点都通过iput()释放，那么理应count=0的空闲结点应该是干净且无锁的。
//此函数不应该被随便调用，所以被设为static,只被iget调用。
//因为得到的结点有可能被设置为dev和inode_num与inode_table里的结点重复.
static m_inode * get_empty_inode(void){
	m_inode * inode;
	int i;
	
	for(i=0;i<NR_INODE;i++){
		if(!inode_table[i].i_count)
			break;
	}
	if(i==NR_INODE)
		panic("inode_table is empty!(get_empty_inode())");
	
	inode=&inode_table[i];

	memset(inode,0,sizeof(inode));
	inode->i_count=1;
	return inode;
}
#if 0
m_inode * get_pipe_inode(void)
{
	struct m_inode * inode;

	if (!(inode = get_empty_inode()))
		return NULL;
	if (!(inode->i_size=get_free_page())) {
		inode->i_count = 0;
		return NULL;
	}
	inode->i_count = 2;	/* sum of readers/writers */
	PIPE_HEAD(*inode) = PIPE_TAIL(*inode) = 0;
	inode->i_pipe = 1;
	return inode;
}
#endif

m_inode * iget(int dev,int nr)
{
	m_inode * inode, * empty;

	if (!dev)
		panic("iget with dev==0");
	
	inode = inode_table;
	while (inode < NR_INODE+inode_table) {
		if (inode->i_dev != dev || inode->i_num != nr) {
			inode++;
			continue;
		}
		wait_on_inode(inode);
		if (inode->i_dev != dev || inode->i_num != nr) {
			inode = inode_table;
			continue;
		}

		inode->i_count++;
		if (inode->i_mount) {
			int i;
			for (i = 0 ; i<NR_SUPER ; i++)
				if (super_block[i].s_imount==inode)
					break;
			if (i >= NR_SUPER) {
				printk("Mounted inode hasn't got sb\n");
				if (empty)
					iput(empty);
				return inode;
			}
			iput(inode);
			dev = super_block[i].s_dev;
			nr = ROOT_INO;
			inode = inode_table;
			continue;
		}
		return inode;
	}
	empty = get_empty_inode();
	if (!empty)
		return NULL;
	inode=empty;
	inode->i_dev = dev;
	inode->i_num = nr;
	read_inode(inode);
	return inode;
}



