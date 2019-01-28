#include<linux/kernel.h>
#include<sys/stat.h>
#include<linux/fs.h>
#include<errno.h>
#include<asm/segment.h>
#include<linux/sched.h>
////���ƽ����Ϣ���û�������
static void cp_stat (struct m_inode *inode, struct stat *statbuf)
{
	struct stat tmp;
	int i;
	//��֤�û�������
	verify_area (statbuf, sizeof (*statbuf));
	
	//��ʱ������Ӧ�ڵ��ϵ���Ϣ��
	tmp.st_dev = inode->i_dev;		// �ļ����ڵ��豸�š�
	tmp.st_ino = inode->i_num;		// �ļ�i �ڵ�š�
	tmp.st_mode = inode->i_mode;	// �ļ����ԡ�
	tmp.st_nlink = inode->i_links_count;	// �ļ�����������
	tmp.st_uid = inode->i_uid;		// �ļ����û�id��
	tmp.st_gid = inode->i_gid;		// �ļ�����id��
	tmp.st_rdev = inode->i_zone[0];	// �豸��(����ļ���������ַ��ļ�����ļ�)��
	tmp.st_size = inode->i_size;	// �ļ���С���ֽ�����������ļ��ǳ����ļ�����
	tmp.st_atime = inode->i_atime;	// ������ʱ�䡣
	tmp.st_mtime = inode->i_mtime;	// ����޸�ʱ�䡣
	tmp.st_ctime = inode->i_ctime;	// ���ڵ��޸�ʱ�䡣
	
	//����Щ״̬��Ϣ���Ƶ��û���������
	for (i = 0; i < sizeof (tmp); i++)
		put_fs_byte (((char *) &tmp)[i], &((char *) statbuf)[i]);
}

//// �ļ�״̬ϵͳ���ú��� - �����ļ�����ȡ�ļ�״̬��Ϣ��
int sys_stat (char *filename, struct stat *statbuf)
{
	m_inode *inode;

	// �����ļ����ҵ���Ӧ��㡣
	if (!(inode = namei (filename)))
		return -ENOENT;
	
	// ���ƽ����Ϣ���û���������
	cp_stat (inode, statbuf);
	iput (inode);
	return 0;
}

//// �ļ�״̬ϵͳ���� - �����ļ������ȡ�ļ�״̬��Ϣ��
int sys_fstat (unsigned int fd, struct stat *statbuf)
{
	file *f;
	m_inode *inode;
	
	// �ļ������Ч
	if (fd >= NR_OPEN || !(f = current->filp[fd]) || !(inode = f->f_inode))
		return -EBADF;
	
	// ���ƽ����Ϣ���û���������
	cp_stat (inode, statbuf);
	return 0;
}

