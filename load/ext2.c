#include<ext2.h>
#include<asm/io.h>
#include<asm/ibmpc.h>
#include<asm/ide.h>
#include<vga.h>
extern char buffer0[1024];
extern char buffer1[1024];

static s8 ide_read_block(char *addr,u32 block,u8 count);
static ext2_dir_entry_2 * find_entry(d_inode * dir,char * name);
static s8 match(char *str0,char *str1,int len);
static int bmap(d_inode * inode,int block);


s8 ext2_load_file(void *loadAddr,char *fileName)
{
	d_super_block *sb;
	group_desc *gd;
	d_inode *inode;
	ext2_dir_entry_2 *entry;
	int inode_table,block,inode_num,file_size,i;

	printf("try to load %s in:0x%x\n",fileName,(u32)loadAddr);
	//读入超级块验证文件系统
	if(ide_read_block(buffer0,1,1)<0)
		return -1;
	sb=(d_super_block*)buffer0;
	if(sb->s_magic!=SUPER_MAGIC)
		return -1;
	printf("superblock magic:%x verify ok\n",SUPER_MAGIC);
	//group信息
	if(ide_read_block(buffer0,2,1)<0)
		return -1;
	gd=(group_desc*)buffer0;

	inode_table=gd->bg_inode_table;
	printf("inode table at:%dblock\n",inode_table);

	if(ide_read_block(buffer0,inode_table,1)<0)
		return -1;
	inode=(d_inode*)buffer0+1;//指向根目录

	if(!(entry=find_entry(inode,fileName)))
		return -1;
	printf("%s inode_num:%d ",fileName,entry->inode);

	inode_num=entry->inode;
	block=(inode_num-1)/8+inode_table;
	if(ide_read_block(buffer0,block,1)<0)
		return -1;
	inode=(d_inode*)buffer0+(inode_num-1)%8;
	file_size=inode->i_size;
	printf("length:%d\n",file_size);
	
	for(i=0;file_size>0;i++,file_size-=BLOCK_SIZE)
	{	
		printf(".");
		block=bmap(inode,i);
		if(block)
			ide_read_block(loadAddr,block,1);
		loadAddr+=1024;
	}
	printf("ok\n");
	return 0;
}
static s8 match(char *str0,char *str1,int len)
{
	int i;
	for(i=0;i<len;i++)
		if(str0[i]!=str1[i])
			return -1;
	return 0;
}
static ext2_dir_entry_2 * find_entry(d_inode * dir,char * name)
{
	int block,i,size;
	ext2_dir_entry_2 * de;

	if(!name)
		return NULL;
	size=(dir)->i_size;
	
	for(i=0; size; i++,size-=BLOCK_SIZE){
		if(!(block=bmap(dir,i++)))//文件空洞
			continue;
		
		if(ide_read_block(buffer1,block,1)<0)
			return NULL;
		de = (ext2_dir_entry_2 *) buffer1;
		
		while(1){
			if(!match(de->name,name,de->name_len))//如果匹配，则返回含目录项的缓存页
				return de;
			
			de=(struct ext2_dir_entry_2*)((unsigned int)de+de->rec_len);
			/*	如果目录项为空 或者 此页已检索完，则继续读入下一页目录项	*/
			if( (!de->rec_len) || \
				(unsigned int )de+de->rec_len > (unsigned int)buffer1+BLOCK_SIZE )
				break;
		}
	}
	return NULL;
}



//取文件数据块对应的磁盘数据块
//12直接、1间接、1双间接、1三间接
//0:表示无数据
static int bmap(d_inode * inode,int block) 
{
	int i;
	if (block<0||block >= 12+256+256*256+256*256*256)
		return -1;
	if (block<12) 
		return inode->i_zone[block];
	
	block -= 12;
	if (block<256) {
		if (!inode->i_zone[12])
			return 0;
		if(ide_read_block(buffer1,inode->i_zone[12],1)<0)
			return -1;
		i = ((unsigned int *) (buffer1))[block];
			return i;
	}
	
	block -= 256;
	if(block<256*256){
		if (!inode->i_zone[13])
			return 0;
		if(ide_read_block(buffer1,inode->i_zone[13],1)<0)
			return -1;
		i = ((unsigned int *)buffer1)[block>>8];		
		if (!i)
			return 0;
		if(ide_read_block(buffer1,i,1)<0)
			return -1;	
		i = ((unsigned int *)buffer1)[block&255];
		return i;
	}
	
	block-=256*256;
	if (!inode->i_zone[14])
		return 0;
	if(ide_read_block(buffer1,inode->i_zone[14],1)<0)
			return -1;	
	i = ((unsigned int *)buffer1)[block>>16];
	if (!i)
		return 0;
	if(ide_read_block(buffer1,i,1)<0)
			return -1;	
	i = ((unsigned int *)buffer1)[(block>>8)&255];
	if(!i)
		return 0;
		if(ide_read_block(buffer1,i,1)<0)
			return -1;	
	i = ((unsigned int *)buffer1)[block&255];
	return i;
}













//block为逻辑扇区(lba)号,count为读扇区数(0表示256)
//		注意:此处block以及count指逻辑扇区，因此为物理扇区2倍
s8 ide_read_block(char *addr,u32 block,u8 count)
{
	block*=2;
	count*=2;
	if(block&0xf0000000)
		return -1;
	/*			arg		*/
	outb(0,MASTER_IDE+FEATURES);
	outb(count,MASTER_IDE+SECTOR_COUNT);	//sector counts
	outb((u8)block,MASTER_IDE+LBALO);				//lba 0~7
	outb((u8)(block>>8),MASTER_IDE+LBAMID);		//lba 8~15
	outb((u8)(block>>16),MASTER_IDE+LBAHI);		//lba 16~23
	outb(0xa0|MODE_LBA|(u8)(block>>24),MASTER_IDE+DRIVEANDHEAD);	//LBA mode,MASTER_IDE,lba 24~27
	outb(READ,MASTER_IDE+COMMAND);				//read operate

	/*	wait for ready	*/
	u8 status;
	do{
		status=inb(MASTER_IDE+STATUS);
	}while(!(status&DRQ_STAT));
	
	/*	read data		*/
	insw(MASTER_IDE+DATA,addr,count*256);//读取字数(word)为count*256
	return 0;
}
