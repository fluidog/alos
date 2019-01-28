/*
 *  linux/fs/bitmap.c
 *
 *  (C) 1991  Linus Torvalds
 */

/* bitmap.c contains the code that handles the inode and block bitmaps */
#include <string.h>

#include <linux/sched.h>
#include <linux/kernel.h>

#define clear_block(addr) \
__asm__ __volatile__ ("cld\n\t" \
	"rep\n\t" \
	"stosl" \
	::"a" (0),"c" (BLOCK_SIZE/4),"D" ((long) (addr)))

#define set_bit(nr,addr) ({\
register int res ; \
__asm__ __volatile__("btsl %2,%3\n\tsetb %%al": \
"=a" (res):"0" (0),"r" (nr),"m" (*(addr))); \
res;})


#define clear_bit(nr,addr) ({\
register int res ; \
__asm__ __volatile__("btrl %2,%3\n\tsetnb %%al": \
"=a" (res):"0" (0),"r" (nr),"m" (*(addr))); \
res;})
/*
#define find_first_zero(addr) ({ \
int __res; \
__asm__ __volatile__ ("cld\n" \
	"1:\tlodsl\n\t" \
	"notl %%eax\n\t" \
	"bsfl %%eax,%%edx\n\t" \
	"je 2f\n\t" \
	"addl %%edx,%%ecx\n\t" \
	"jmp 3f\n" \
	"2:\taddl $32,%%ecx\n\t" \
	"cmpl $8192,%%ecx\n\t" \
	"jl 1b\n" \
	"3:" \
	:"=c" (__res):"c" (0),"S" (addr)); \
__res;})*/

unsigned int find_first_zero(char *addr)
{
	if(!addr)
		panic("null point");
	int res=0;
	while(*addr==(char)0xff){
		addr++;
		res+=8;
	}	
	while(*addr%2)
	{
		res++;
		*addr=*addr/2;
	}	
	return res;
}

void free_block(int dev, int block)
{
	m_super_block * sb;
	buffer_head * bh;
	group_desc *gd;
	int i,j,groups,block,off;

	if (!(sb = get_super(dev)))
		panic("trying to free block on nonexistent device");
	
	if (block < sb->s_first_data_block || block >= sb->s_blocks_count)
		panic("trying to free block not in datazone");
	
	groups=block / sb->s_blocks_per_group;
	off=block % sb->s_inodes_per_group;
	
	if(!(bh=get_group(sb,groups,&gd)))
			panic("trying to free block without group information");
	
	if (clear_bit(off,sb->s_zmap[groups])){
		printk("free_block: bit already cleared\n");
	}else{
		sb->s_free_blocks_count++;	//������ʣ����
		gd->bg_free_blocks_count++; //���ʣ����
		bh->b_dirt=1;
	}
	brelse(bh);
	sb->s_zmap[groups].b_dirt=1;
}

buffer_head* new_block(int dev)
{
	buffer_head * bh;
	m_super_block * sb;
	group_desc *gd;
	int i,j,groups,block;
	

	if (!(sb = get_super(dev)))
		panic("trying to get new block from nonexistant device");
	if(!sb->s_free_blocks_count)
		panic("no free blocks");
	
	groups=sb->s_inodes_count / sb->s_inodes_per_group;
	for(i=0;i<groups;i++){
		if(!(bh=get_group(sb,i,&gd))){
			return NULL;
		}
		if(!gd->bg_free_blocks_count)//��ʣ��ҳ��
			continue;


		sb->s_free_blocks_count--;//������ʣ����-1
		gd->bg_free_blocks_count--;//���ʣ����-1
		bh->b_dirt=1;
		brelse(bh);

		bh=sb->s_zmap[i];
		
		block=find_first_zero(bh->b_data);
		if(set_bit(block,bh->b_data))
			panic("new_block: bit already set,system not confirm");
		bh->b_dirt=1;

		block=block+i*sb->s_blocks_per_group;
		if(!(bh=bread(dev,block)))
			return NULL;
		return bh;
		
	}
	panic("system error");
}

void free_inode(m_inode * inode)
{
	m_super_block * sb;
	buffer_head * bh;
	group_desc *gd;
	int groups,off;
	if (!inode)
		return;
	if (!inode->i_dev) {
		memset(inode,0,sizeof(*inode));
		return;
	}
	if (inode->i_count>1) {
		printk("trying to free inode with count=%d\n",inode->i_count);
		panic("free_inode");
	}
	if (inode->i_links_count)
		panic("trying to free inode with links");
	if (!(sb = get_super(inode->i_dev)))
		panic("trying to free inode on nonexistent device");
	if (inode->i_num < sb->s_first_ino || inode->i_num > sb->s_inodes_count)
		panic("trying to free inode 0 or nonexistant inode");

	groups=(inode->i_num-1) / sb->s_inodes_per_group;
	off=(inode->i_num-1) % sb->s_inodes_per_group;
	if(!(bh=get_group(sb,groups,&gd)))
		panic("trying to free inode without group information");
	if (clear_bit(off,sb->s_imap[groups])){
		printk("free_inode: bit already cleared\n");
	}else{
		sb->s_free_inodes_count++;	//������ʣ����
		gd->bg_free_inodes_count++;	//���ʣ����
		bh->b_dirt=1;
	}
	brelse(bh);
	sb->s_imap[groups].b_dirt=1;
}

m_inode * new_inode(int dev)
{	
	m_inode * inode;
	m_super_block * sb;
	buffer_head * bh;
	group_desc *gd;
	int i,j,groups,block,inode_num;
	
	if (!(inode=get_empty_inode()))
		return NULL;
	if (!(sb = get_super(dev)))
		panic("new_inode with unknown device");
	if(!sb->s_free_inodes_count)
		panic("no free inode");
	
	groups=sb->s_inodes_count / sb->s_inodes_per_group;
	for(i=0;i<groups;i++){
		if(!(bh=get_group(sb,i,&gd))){
			iput(inode);
			return NULL;
		}
		if(!gd->bg_free_inodes_count)//��ʣ����
			continue;
		
		sb->s_free_inodes_count--;//������ʣ����-1
		gd->bg_free_inodes_count--;//���ʣ����-1
		bh->b_dirt=1;
		brelse(bh);

		bh=sb->s_imap[i];
		inode_num=find_first_zero(bh->b_data);
		if(set_bit(inode_num,bh->b_data))
			panic("new_inode: bit already set,system not confirm");
		bh->b_dirt=1;
		
		inode->i_count=1;
		inode->i_links_count=1;	
		inode->i_uid=current->euid;
		inode->i_gid=current->egid;
		inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
		inode->i_dev=dev;
		inode->i_dirt=1;
		inode->i_num=inode_num+1+i*sb->s_inodes_per_group;		
		return inode;
	}
	panic("system error");
}
