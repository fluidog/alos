/*
 *  linux/fs/block_dev.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <errno.h>

#include <linux/fs.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <asm/system.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

int block_write(int dev, long * pos,  char * buf, int count)
{
	int block,offset,chars,left;
	buffer_head * bh;
	register char * p;

	left=count;
	if ((left=count)<=0)
		return 0;
	
	while (left) {
		block = *pos >> BLOCK_SIZE_BITS;	//(*pos)/BLOCK_SIZE
		offset = *pos & (BLOCK_SIZE-1);		//(*pos)%BLOCK_SIZE
		chars=MIN(BLOCK_SIZE-offset,left);

		if(chars==BLOCK_SIZE){		//整页内容都将被覆盖，所以无需读磁盘
			if(!(bh=getblk(dev,block)))
				return (count-left)?(count-left):-EIO;
		}
		else{
			if (!(bh = breada(dev,block,block+1,block+2,-1)))
				return (count-left)?(count-left):-EIO;
		}
		
		p = offset + bh->b_data;
		*pos += chars;
		left -= chars;
		while (chars--)
			*(p++) = get_fs_byte(buf++);
		brelse(bh);
	}
	return (count-left)?(count-left):-EIO;
}

int block_read(int dev, unsigned long * pos, char * buf, int count)
{
	int block,offset,chars,left;
	buffer_head * bh;
	register char * p;

	left=count;
	if ((left=count)<=0)
		return 0;
	
	while (left) {
		block = *pos >> BLOCK_SIZE_BITS;	//(*pos)/BLOCK_SIZE
		offset = *pos & (BLOCK_SIZE-1);		//(*pos)%BLOCK_SIZE
		chars=MIN(BLOCK_SIZE-offset,left);

		if (!(bh = breada(dev,block,block+1,block+2,-1)))
			return (count-left)?(count-left):-EIO;
		p = offset + bh->b_data;
		*pos += chars;
		left -= chars;
		while (chars--)
			put_fs_byte(*(p++),buf++);
		brelse(bh);
	}
	return (count-left)?(count-left):-EIO;
}
