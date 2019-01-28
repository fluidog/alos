#include<linux/kernel.h>
#include<sys/stat.h>
#include<linux/fs.h>
#include<errno.h>
#include<asm/segment.h>
#include<linux/sched.h>
////复制结点信息到用户缓冲区
static void cp_stat (struct m_inode *inode, struct stat *statbuf)
{
	struct stat tmp;
	int i;
	//验证用户缓冲区
	verify_area (statbuf, sizeof (*statbuf));
	
	//临时复制相应节点上的信息。
	tmp.st_dev = inode->i_dev;		// 文件所在的设备号。
	tmp.st_ino = inode->i_num;		// 文件i 节点号。
	tmp.st_mode = inode->i_mode;	// 文件属性。
	tmp.st_nlink = inode->i_links_count;	// 文件的连接数。
	tmp.st_uid = inode->i_uid;		// 文件的用户id。
	tmp.st_gid = inode->i_gid;		// 文件的组id。
	tmp.st_rdev = inode->i_zone[0];	// 设备号(如果文件是特殊的字符文件或块文件)。
	tmp.st_size = inode->i_size;	// 文件大小（字节数）（如果文件是常规文件）。
	tmp.st_atime = inode->i_atime;	// 最后访问时间。
	tmp.st_mtime = inode->i_mtime;	// 最后修改时间。
	tmp.st_ctime = inode->i_ctime;	// 最后节点修改时间。
	
	//将这些状态信息复制到用户缓冲区中
	for (i = 0; i < sizeof (tmp); i++)
		put_fs_byte (((char *) &tmp)[i], &((char *) statbuf)[i]);
}

//// 文件状态系统调用函数 - 根据文件名获取文件状态信息。
int sys_stat (char *filename, struct stat *statbuf)
{
	m_inode *inode;

	// 根据文件名找到对应结点。
	if (!(inode = namei (filename)))
		return -ENOENT;
	
	// 复制结点信息到用户缓冲区。
	cp_stat (inode, statbuf);
	iput (inode);
	return 0;
}

//// 文件状态系统调用 - 根据文件句柄获取文件状态信息。
int sys_fstat (unsigned int fd, struct stat *statbuf)
{
	file *f;
	m_inode *inode;
	
	// 文件句柄有效
	if (fd >= NR_OPEN || !(f = current->filp[fd]) || !(inode = f->f_inode))
		return -EBADF;
	
	// 复制结点信息到用户缓冲区。
	cp_stat (inode, statbuf);
	return 0;
}

