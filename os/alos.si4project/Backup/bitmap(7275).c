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
/*ֻ�������block bit map��Ӧλ���ɣ�ҳ�����ݲ�����,����ҳ��Ļ���ҳҲ����ı�,
��Ϊ����ʹ��ҳ��δ��ʹ�ã���ҳ����Ȼ���Զ����ڴ棬����δʹ�õ�ҳ��Ҳ��������������Ϣ*/
void free_block(int dev, int block)
{
	m_super_block * sb;
	buffer_head * group_bh,*zmap_bh;
	group_desc *gd;
	int group,off;

	if (!(sb = get_super(dev)))
		panic("trying to free block on nonexistent device");
	
	if (block < sb->s_first_data_block || block >= sb->s_blocks_count)
		panic("trying to free block not in datazone");
	
	group=block / sb->s_blocks_per_group;
	off=block % sb->s_inodes_per_group;
	
	if(!(group_bh=get_group(sb,group,&gd)))
		panic("trying to free block without group information");
	if(!(zmap_bh=get_zmap(sb,group)))
		panic("trying to free block without zmap information");
	if (clear_bit(off,zmap_bh->b_data))
		printk("free_block: bit already cleared\n");
	
	sb->s_free_blocks_count++;	//������ʣ����
	gd->bg_free_blocks_count++; //���ʣ����
	
	group_bh->b_dirt=1;
	zmap_bh->b_dirt=1;	
	brelse(group_bh);
	brelse(zmap_bh);
	return ;
}

buffer_head* new_block(int dev)
{
	buffer_head * group_bh,*zmap_bh,*bh;
	m_super_block * sb;
	group_desc *gd;
	int i,groups,block,off;
	
	if (!(sb = get_super(dev)))
		panic("trying to get new block from nonexistant device");
	if(!sb->s_free_blocks_count)
		panic("no free blocks");
	
	groups=sb->s_inodes_count / sb->s_inodes_per_group;
	for(i=0;i<groups;i++){
		if(!(group_bh=get_group(sb,i,&gd))){
			return NULL;
		}
		if(!gd->bg_free_blocks_count){//��ʣ��ҳ��
			brelse(group_bh);
			continue;
		}
		if(!(zmap_bh=get_zmap(sb,i))){
			brelse(group_bh);
			return NULL;
		}
		off=find_first_zero(zmap_bh->b_data);
		block=off+i*sb->s_blocks_per_group;
		/*��ʹҳδʹ�ã�Ҳ��������ʶ���ҳ��δʹ�õ�ҳ����Ҳ��һ��Ϊ0,
		��ˣ���Ϊ��ҳʹ���轫������*/
		if(!(bh=bread(dev,block))){
			brelse(group_bh);
			brelse(group_bh);
			return NULL;
		}
		memset(bh->b_data,0,sizeof(BLOCK_SIZE));
		if(set_bit(block,zmap_bh->b_data))
			panic("new_block: bit already set,system not confirm");
	
		sb->s_free_blocks_count--;//������ʣ����-1
		gd->bg_free_blocks_count--;//���ʣ����-1
		
		group_bh->b_dirt=1;
		zmap_bh->b_dirt=1;
		bh->b_dirt=1;
		return bh;
	}
	panic("system error");
}


/*���ͷŽ�㺯��ֻ�������������Ӧλ����ǽ��δ�ã������������ʵ������,
���ң��Ὣɾ����Ϣ��ӵ������У�
Ҳ�����������������ļ�ռ��block,��ˣ�������ǰ��֤�ļ�����Ϊ0*/
void free_inode(m_inode * inode)
{
	m_super_block * sb;
	buffer_head * group_bh,*imap_bh;
	group_desc *gd;
	int group,off;
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

	group=(inode->i_num-1) / sb->s_inodes_per_group;
	off=(inode->i_num-1) % sb->s_inodes_per_group;
	if(!(group_bh=get_group(sb,group,&gd)))
		panic("trying to free inode without group information");
	if(!(imap_bh=get_imap(sb,group)))
		panic("trying to free inode without iamp information");
	if (clear_bit(off,imap_bh->b_data))
		printk("free_inode: bit already cleared\n");
	
	sb->s_free_inodes_count++;	//������ʣ����
	gd->bg_free_inodes_count++;	//���ʣ����
	group_bh->b_dirt=1;
	imap_bh->b_dirt=1;
	inode->i_dtime=CURRENT_TIME;
	inode->i_dirt=1;
	brelse(group_bh);
	brelse(imap_bh);
	return ;
		
}

/*�õ���inode(�ļ�)����Ϊ0��i_zone[]ҲȫΪ0,
�˺��������������ʵ�����ݣ�ֱ����Ϊ0,Ȼ����������Ӧ����
����ɾ��һ����㣬����ɾ���������ݣ���Ϊ�ָ�������*/
m_inode * new_inode(int dev)
{	
	m_inode * inode;
	m_super_block * sb;
	buffer_head * group_bh,*imap_bh;
	group_desc *gd;
	int i,groups,inode_num,off;
	
	if (!(sb = get_super(dev)))
		panic("new_inode with unknown device");
	if(!sb->s_free_inodes_count)
		panic("no free inode");
	
	groups=sb->s_inodes_count / sb->s_inodes_per_group;
	for(i=0;i<groups;i++){
		if(!(group_bh=get_group(sb,i,&gd)))
			return NULL;
		
		if(!gd->bg_free_inodes_count){//��ʣ����
			brelse(group_bh);
			continue;
		}	
		if(!(imap_bh=get_imap(sb,i))){
			brelse(group_bh);
			return NULL;
		}
			
		off=find_first_zero(imap_bh->b_data);
		inode_num=off+(1+i*sb->s_inodes_per_group);//ʵ�ʽ���
		/*�˺������������ʶ��˽ڵ�,������˽�㣬��ʹ���map���δʹ��,��Ȼ���Զ��������Ϣ
			����δʹ�õĽ���������������Ҳ��һ��Ϊ0����ˣ��轫�õ��Ľ������*/
		if(!(inode=iget(dev,inode_num))){
			brelse(group_bh);
			brelse(imap_bh);
			return NULL;
		}
		memset(inode,0,sizeof(m_inode));
		sb->s_free_inodes_count--;//������ʣ����-1
		gd->bg_free_inodes_count--;//���ʣ����-1
		if(set_bit(off,imap_bh->b_data))
			panic("new_inode: bit already set,system not confirm");
		group_bh->b_dirt=1;
		imap_bh->b_dirt=1;
		brelse(group_bh);
		brelse(imap_bh);
			
		inode->i_count=1;
		inode->i_links_count=1;	
		inode->i_uid=current->euid;
		inode->i_gid=current->egid;
		inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
		inode->i_dev=dev;
		inode->i_num=inode_num;
		inode->i_dirt=1;
		return inode;
	}
	panic("system error");
}


