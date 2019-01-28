/*
 *  linux/fs/inode.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <string.h> 

//#include <linux/sched.h>
#include<linux/fs.h>
#include <linux/kernel.h>
#include <asm/system.h>

struct m_inode inode_table[NR_INODE]={{0,},};

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
/*
void sync_inodes(void)
{
	int i;
	struct m_inode * inode;

	inode = 0+inode_table;
	for(i=0 ; i<NR_INODE ; i++,inode++) {
		wait_on_inode(inode);
		if (inode->i_dirt && !inode->i_pipe)
			write_inode(inode);
	}
}*/
int new_block(unsigned short dev)
{
	panic("new_block not support");
	return 0; 
}

//取文件数据块对应的磁盘数据块
//12直接、1间接、1双间接、1三间接
static int _bmap(struct m_inode * inode,int block,int create)
{
	struct buffer_head * bh;
	int i;
	if (block<0||block >= 12+256+256*256+256*256*256)
		panic("_bmap: block<0 || block>big");
	if (block<12) {
		if (create && !inode->i_zone[block])
			if ((inode->i_zone[block]=new_block(inode->i_dev))) {
				inode->i_ctime=CURRENT_TIME;
				inode->i_dirt=1;
			}
		return inode->i_zone[block];	
	}
	block -= 12;
	if (block<256) {
		if (create && !inode->i_zone[12])
			if ((inode->i_zone[12]=new_block(inode->i_dev))) {
				inode->i_dirt=1;
				inode->i_ctime=CURRENT_TIME;
			}
		if (!inode->i_zone[12])
			return 0;
		if (!(bh = bread(inode->i_dev,inode->i_zone[12])))
			return 0;
		i = ((unsigned int *) (bh->b_data))[block];
		if (create && !i)
			if ((i=new_block(inode->i_dev))) {
				((unsigned int *) (bh->b_data))[block]=i;
				bh->b_dirt=1;
			}
		brelse(bh);
		return i;
	}
	block -= 256;
	if(block<256*256){
		if (create && !inode->i_zone[13])
			if ((inode->i_zone[13]=new_block(inode->i_dev))) {
				inode->i_dirt=1;
				inode->i_ctime=CURRENT_TIME;
			}
		if (!inode->i_zone[13])
			return 0;
		if (!(bh=bread(inode->i_dev,inode->i_zone[13])))
			return 0;
		i = ((unsigned int *)bh->b_data)[block>>8];
		if (create && !i)
			if ((i=new_block(inode->i_dev))) {
				((unsigned int *) (bh->b_data))[block>>8]=i;
				bh->b_dirt=1;
			}
		brelse(bh);
		if (!i)
			return 0;
		if (!(bh=bread(inode->i_dev,i)))
			return 0;
		i = ((unsigned int *)bh->b_data)[block&255];
		if (create && !i)
			if ((i=new_block(inode->i_dev))) {
				((unsigned int *) (bh->b_data))[block&255]=i;
				bh->b_dirt=1;
			}
		brelse(bh);
		return i;
	}
	block-=256*256;
	if (create && !inode->i_zone[14])
		if ((inode->i_zone[14]=new_block(inode->i_dev))) {
			inode->i_dirt=1;
			inode->i_ctime=CURRENT_TIME;
		}
	if (!inode->i_zone[14])
		return 0;
	if (!(bh=bread(inode->i_dev,inode->i_zone[14])))
		return 0;
	i = ((unsigned int *)bh->b_data)[block>>16];
	if (create && !i)
		if ((i=new_block(inode->i_dev))) {
			((unsigned int *) (bh->b_data))[block>>16]=i;
			bh->b_dirt=1;
		}
	brelse(bh);
	if (!i)
		return 0;
	if (!(bh=bread(inode->i_dev,i)))
		return 0;
	i = ((unsigned int *)bh->b_data)[(block>>8)&255];
	if (create && !i)
		if ((i=new_block(inode->i_dev))) {
			((unsigned int *) (bh->b_data))[(block>>8)&255]=i;
			bh->b_dirt=1;
		}
	brelse(bh);
	if(!i)
		return 0;
	if(!(bh=bread(inode->i_dev,i)))
		return 0;
	i=((unsigned int *)bh->b_data)[block&255];
	if(create && !i)
		if((i=new_block(inode->i_dev))){
			((unsigned int*)(bh->b_data))[block&255]=i;
			bh->b_dirt=1;
		}
	brelse(bh);
	return i;
}


int bmap(struct m_inode * inode,int block)
{
	return _bmap(inode,block,0);
}

int create_block(struct m_inode * inode, int block)
{
	return _bmap(inode,block,1);
}
		
void iput(struct m_inode * inode)
{
	if (!inode)
		return;
	
	if (!inode->i_count)
		panic("iput: trying to free free inode");
	
	

	if (inode->i_count>1) {
		inode->i_count--;
		return;
	}
	
	if (inode->i_dirt) {
		//write_inode(inode);	/* we can sleep - so do again */
	}
	inode->i_count--;
	
	return;
}

struct m_inode * get_empty_inode(void)
{
	struct m_inode * inode;
	static struct m_inode * last_inode = inode_table;
	int i;
	for(i=0;i<NR_INODE;i++){
		if(!inode_table[i].i_count)
			break;
	}
	if(i==NR_INODE)
		panic("inode_table is empty!(get_empty_inode())");
	inode=inode_table+i;

	memset(inode,0,sizeof(inode));
	inode->i_count=1;
	return inode;
}
#if 0
struct m_inode * get_pipe_inode(void)
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
extern struct super_block super_block[NR_SUPER];

struct m_inode * iget(int dev,int nr)
{
	struct m_inode * inode, * empty;

	if (!dev)
		panic("iget with dev==0");
	empty = get_empty_inode();
	inode = inode_table;
	while (inode < NR_INODE+inode_table) {
		if (inode->i_dev != dev || inode->i_num != nr) {
			inode++;
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
		if (empty)
			iput(empty);
		return inode;
	}
	if (!empty)
		return NULL;
	inode=empty;
	inode->i_dev = dev;
	inode->i_num = nr;
	read_inode(inode);
	return inode;
}

static void read_inode(struct m_inode * inode)
{
	struct super_block * sb;
	struct buffer_head * bh;
	int block,group;
	struct group_desc *g;

	lock_inode(inode);
	if (!(sb=get_super(inode->i_dev)))
		panic("trying to read inode without dev");


	group=(inode->i_num-1) / sb->s_inodes_per_group;
	
	g=get_group(sb,group);

	block=g->bg_inode_table+(inode->i_num-1) % sb->s_inodes_per_group /INODES_PER_BLOCK;
	if (!(bh=bread(inode->i_dev,block)))
		panic("unable to read i-node block");
	*(struct d_inode *)inode =
		((struct d_inode *)bh->b_data)
			[(inode->i_num-1)%sb->s_inodes_per_group%INODES_PER_BLOCK];
			
	brelse(bh);
	unlock_inode(inode);
}
#if 0
static void write_inode(struct m_inode * inode)
{
	struct super_block * sb;
	struct buffer_head * bh;
	int block;

	lock_inode(inode);
	if (!inode->i_dirt || !inode->i_dev) {
		unlock_inode(inode);
		return;
	}
	if (!(sb=get_super(inode->i_dev)))
		panic("trying to write inode without device");
	block = 2 + sb->s_imap_blocks + sb->s_zmap_blocks +
		(inode->i_num-1)/INODES_PER_BLOCK;
	if (!(bh=bread(inode->i_dev,block)))
		panic("unable to read i-node block");
	((struct d_inode *)bh->b_data)
		[(inode->i_num-1)%INODES_PER_BLOCK] =
			*(struct d_inode *)inode;
	bh->b_dirt=1;
	inode->i_dirt=0;
	brelse(bh);
	unlock_inode(inode);
}
#endif
