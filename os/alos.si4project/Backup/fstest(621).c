#include"fstest.h"
//#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#define ide_read(addr,lba,size) \
	fd=open("../e.img",O_RDONLY);\
	lseek(fd,(lba)*512,SEEK_SET);\
	read(fd,addr,size);\
	close(fd);
void main()
{
	int fd;
	struct ext2_super_block *super_block_p;
	super_block_p=(struct ext2_super_block *)malloc(1024);

	ide_read((char *)super_block_p,2,1024);

	printf("inode:%u block:%u size:%u\n",super_block_p->s_inodes_per_group,\
			super_block_p->s_blocks_per_group,super_block_p->s_log_block_size);

	int group=super_block_p->s_blocks_count/(1024*8)*sizeof(struct ext2_group_desc);
	printf("group:%d\n",group);
	//free(super_block_p);
	//super_block_p=NULL;
	
	struct ext2_group_desc *group_desc_p;
	group_desc_p=(struct ext2_group_desc *)malloc(group);

	ide_read((char*)group_desc_p,2*2,group);

	printf("block_bitmap:%u inode_table:%u\n",group_desc_p->bg_block_bitmap,\
			group_desc_p->bg_inode_table);
	printf("inode:%d\n",sizeof(struct ext2_inode));
	struct ext2_inode *inode_p;
	inode_p=(struct ext2_inode *)malloc(4*sizeof(struct ext2_inode));

	ide_read((char*)inode_p,2*group_desc_p->bg_inode_table,4*sizeof(struct ext2_inode));
	int i;
	for(i=0;i<4;i++)
		printf("%d_i_block:%u\n",i,inode_p[i].i_block[0]);

	struct ext2_dir_entry_2 *entry=malloc(1024);
	ide_read((char*)entry,2*inode_p[1].i_block[0],1024);

	printf("name:%s inode:%d\n",entry->name,entry->inode);
	entry=(unsigned int)entry+entry->rec_len;
	printf("name:%s inode:%d\n",entry->name,entry->inode);
	entry=(unsigned int)entry+entry->rec_len;
	printf("name:%s inode:%d\n",entry->name,entry->inode);


	

		
}
