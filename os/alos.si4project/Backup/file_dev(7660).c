/*
 *  linux/fs/file_dev.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

int file_read(file * filp, char * buf, int count)
{
	int left,chars,nr;
	buffer_head * bh;
	m_inode * inode;
	char * p;
	
	inode=filp->f_inode;
	//读不能超出文件
	if (count+filp->f_pos > inode->i_size)
			count = inode->i_size - filp->f_pos;
	if ((left=count)<=0)
		return 0;
	
	while (left) {
		if ((nr = bmap(inode,(filp->f_pos)/BLOCK_SIZE))) {
			if (!(bh=bread(inode->i_dev,nr)))
				break;
		} else
			bh = NULL;
		nr = filp->f_pos % BLOCK_SIZE;
		chars = MIN( BLOCK_SIZE-nr , left );
		filp->f_pos += chars;//文件定位符
		left -= chars;//剩余字节
		if (bh) {
			p = nr + bh->b_data;
			while (chars-->0)
				put_fs_byte(*(p++),buf++);
			brelse(bh);
		} else {
			while (chars-->0)
				put_fs_byte(0,buf++);
		}
	}
	inode->i_atime = CURRENT_TIME;
	return (count-left)?(count-left):-ERROR;
}

int file_write(file * filp, char * buf, int count)
{
	int left,chars,nr;
	int block;
	buffer_head * bh;
	char * p;
	m_inode * inode;
	
	inode=filp->f_inode;
/*
 * ok, append may not work when many processes are writing at the same time
 * but so what. That way leads to madness anyway.
 */
	if (filp->f_flags & O_APPEND)
		filp->f_pos = inode->i_size;
	if ((left=count)<=0)
		return 0;
	
	while (left) {
		if (!(block = create_block(inode,(filp->f_pos)/BLOCK_SIZE)))
			break;
		if (!(bh=bread(inode->i_dev,block)))
			break;
		bh->b_dirt=1;
		nr=filp->f_pos%BLOCK_SIZE;
		chars=MIN(BLOCK_SIZE-nr,left);
		filp->f_pos+=chars;
		left-=chars;
		p=bh->b_data+nr;
		
		while(chars--)
			*(p++) = get_fs_byte(buf++);
		brelse(bh);
	}
	if(filp->f_pos>inode->i_size){
		inode->i_size=filp->f_pos;
		inode->i_ctime=CURRENT_TIME;
	}
	inode->i_mtime=CURRENT_TIME;
	return (count-left)?(count-left):-ERROR;
}
