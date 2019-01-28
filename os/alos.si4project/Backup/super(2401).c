/*
 *  linux/fs/super.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * super.c contains code to handle the super-block tables.
 */
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/system.h>
#include<string.h>
#include <errno.h>
#include <sys/stat.h>



m_super_block super_block[NR_SUPER];
/* this is initialized in init/main.c */
int ROOT_DEV;
buffer_head* get_group(m_super_block *sb,int group_num,group_desc **gd)
{
	buffer_head *bh;
	int block,groups;
	group_desc *g;
	if(!sb)
		return NULL;
	groups=sb->s_inodes_count / sb->s_inodes_per_group;	//group总数
	if(group_num>=groups || group_num<0)
		return NULL;
	
	block=group_num/GROUPS_PER_BLOCK+2;
	if(!(bh=bread(sb->s_dev,block)))
		return NULL;
	if(gd)
		*gd=(group_desc *)bh->b_data + group_num%GROUPS_PER_BLOCK;
	return bh;
}
//获得第group_num组的inode bitmap
buffer_head *get_imap(m_super_block *sb,int group_num)
{
	group_desc *gd;
	buffer_head *bh;
	int block;
	if(!(bh=get_group(sb,group_num,&gd)))
		return NULL;
	block=gd->bg_inode_bitmap;
	brelse(bh);

	if(!(bh=bread(sb->s_dev,block)))
		return NULL;
	return bh;
}
//获得第group_num组的block bitmap
buffer_head *get_zmap(m_super_block *sb,int group_num)
{
	group_desc *gd;
	buffer_head *bh;
	int block;
	if(!(bh=get_group(sb,group_num,&gd)))
		return NULL;
	block=gd->bg_block_bitmap;
	brelse(bh);

	if(!(bh=bread(sb->s_dev,block)))
		return NULL;
	return bh;

}


static void lock_super(m_super_block * sb)
{
	cli();
	while (sb->s_lock)
		sleep_on(&(sb->s_wait));
	sb->s_lock = 1;
	sti();
}

static void free_super(m_super_block * sb)
{
	cli();
	sb->s_lock = 0;
	wake_up(&(sb->s_wait));
	sti();
}

static void wait_on_super(m_super_block * sb)
{
	cli();
	while (sb->s_lock)
		sleep_on(&(sb->s_wait));
	sti();
}

m_super_block * get_super(int dev)
{
	m_super_block * s;

	if (!dev)
		return NULL;
	s = 0+super_block;
	while (s < NR_SUPER+super_block)
		if (s->s_dev == dev) {
			wait_on_super(s);
			if (s->s_dev == dev)
				return s;
			s = 0+super_block;
		} else
			s++;
	return NULL;
}
void sync_super(void)
{
	int i;
	m_super_block *sb;
	buffer_head *bh;
	sb=0+super_block;
	while(sb<super_block +NR_SUPER){
		if(sb->s_dev){
			lock_super(sb);
			if(bh=bread(sb->s_dev,1)){
				//超级块磁盘数据
				*((d_super_block *) bh->b_data) =  *((d_super_block *) sb);
				bh->b_dirt=1;
				brelse(bh);
			}
			free_super(sb);
		}
		sb++;
	}
}

static void put_super(int dev)
{
	m_super_block * sb;
	/* struct m_inode * inode;*/
	int i;
	buffer_head *bh;
	if (!(sb = get_super(dev)))
		return;
	if (sb->s_imount) {
		printk("Mounted disk changed - tssk, tssk\n\r");
		return;
	}
	lock_super(sb);
	if (!(bh = bread(dev,1))) 
			panic("put_super()");
	
	//超级块磁盘数据
	*((d_super_block *) bh->b_data) = \
		*((d_super_block *) sb);
	bh->b_dirt=1;
	
	brelse(bh);

	if (dev != ROOT_DEV){
		printk ("root diskette changed: prepare for armageddon\n\r");
		free_super(sb);
		return ;
	}
	sb->s_dev=0;
	free_super(sb);
	return;
}

static m_super_block * read_super(int dev)
{
	m_super_block * s;
	buffer_head * bh;
	group_desc *group;
	int groups;
	int i,block;

	if (!dev)
		return NULL;

	if ((s = get_super(dev)))
		return s;
	for (s = 0+super_block ;; s++) {
		if (s >= NR_SUPER+super_block)
			return NULL;
		if (!s->s_dev)//0号设备不存在
			break;
	}
	

	memset(s,0,sizeof(m_super_block));
	
	s->s_dev = dev;			//dev
	lock_super(s);
	if (!(bh = bread(dev,1))) {
		s->s_dev=0;
		free_super(s);
		return NULL;
	}
	*((d_super_block *) s) =		//超级块磁盘数据
		*((d_super_block *) bh->b_data);
	brelse(bh);
	if (s->s_magic != SUPER_MAGIC) {	//验证魔数,确认文件系统
		s->s_dev = 0;
		free_super(s);
		return NULL;
	}
		
	free_super(s);
	return s;
}

int sys_umount(char * dev_name)
{
	m_inode * inode;
	m_super_block * sb;
	int dev;

	if (!(inode=namei(dev_name)))
		return -ENOENT;
	dev = inode->i_zone[0];
	if (!S_ISBLK(inode->i_mode)) {
		iput(inode);
		return -ENOTBLK;
	}
	iput(inode);
	if (dev==ROOT_DEV)
		return -EBUSY;
	if (!(sb=get_super(dev)) || !(sb->s_imount))
		return -ENOENT;
	if (!sb->s_imount->i_mount)
		printk("Mounted inode has i_mount=0\n");
	for (inode=inode_table+0 ; inode<inode_table+NR_INODE ; inode++)
		if (inode->i_dev==dev && inode->i_count)
				return -EBUSY;
	sb->s_imount->i_mount=0;
	iput(sb->s_imount);
	sb->s_imount = NULL;
	iput(sb->s_isup);
	sb->s_isup = NULL;
	put_super(dev);
	sync_dev(dev);
	return 0;
}

int sys_mount(char * dev_name, char * dir_name)
{
	m_inode * dev_i, * dir_i;
	m_super_block * sb;
	int dev;

	if (!(dev_i=namei(dev_name)))
		return -ENOENT;
	dev = dev_i->i_zone[0];
	if (!S_ISBLK(dev_i->i_mode)) {
		iput(dev_i);
		return -EPERM;
	}
	iput(dev_i);
	if (!(dir_i=namei(dir_name)))
		return -ENOENT;
	if (dir_i->i_count != 1 || dir_i->i_num == ROOT_INO) {
		iput(dir_i);
		return -EBUSY;
	}
	if (!S_ISDIR(dir_i->i_mode)) {
		iput(dir_i);
		return -EPERM;
	}
	if (!(sb=read_super(dev))) {
		iput(dir_i);
		return -EBUSY;
	}
	if (sb->s_imount) {
		iput(dir_i);
		return -EBUSY;
	}
	if (dir_i->i_mount) {
		iput(dir_i);
		return -EPERM;
	}
	sb->s_imount=dir_i;
	dir_i->i_mount=1;
	dir_i->i_dirt=1;		/* NOTE! we don't iput(dir_i) */
	return 0;			/* we do that in umount */
}

void mount_root(void)
{
	int i,free;
	m_super_block * sb;
	m_inode * mi;

	if (MAJOR(ROOT_DEV) == 2) {
		printk("Insert root floppy and press ENTER");
		//wait_for_keypress();
	}
	
	memset(file_table,0,sizeof(file_table));
	memset(super_block,0,sizeof(super_block));
	memset(inode_table,0,sizeof(inode_table));

	if (!(sb=read_super(ROOT_DEV)))
		panic("Unable to mount root");

	if (!(mi=iget(ROOT_DEV,ROOT_INO)))
		panic("Unable to read root i-node");
	mi->i_count += 3 ;	/* NOTE! it is logically used 4 times, not 1 */
	sb->s_isup = mi;
	current->pwd = mi;
	current->root = mi;
}

