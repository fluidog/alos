/*
 *  linux/fs/namei.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * Some corrections by tytso.
 */

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include<linux/fs.h>
#include <string.h> 
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#define ACC_MODE(x) ("\004\002\006\377"[(x)&O_ACCMODE])
file file_table[NR_FILE];

/*
 * comment out this line if you want names > NAME_LEN chars to be
 * truncated. Else they will be disallowed.
 */
/* #define NO_TRUNCATE */

#define MAY_EXEC 1
#define MAY_WRITE 2
#define MAY_READ 4

/*
 *	permission()
 *
 * is used to check for read/write/execute permissions on a file.
 * I don't know if we should look at just the euid or both euid and
 * uid, but that should be easily changed.
 */
static int permission(m_inode * inode,int mask)
{
	int mode = inode->i_mode;
/* special case: not even root can read/write a deleted file */
	if (inode->i_dev && !inode->i_links_count)
		return 0;
	else if (current->euid==inode->i_uid)
		mode >>= 6;
	else if (current->egid==inode->i_gid)
		mode >>= 3;
	if (((mode & mask & 0007) == mask) || suser())
		return 1;
	return 0;
}

/*
 * ok, we cannot use strncmp, as the name is not in our data space.
 * Thus we'll have to use match. No big problem. Match also makes
 * some sanity tests.
 *
 * NOTE! unlike strncmp, match returns 1 for success, 0 for failure.
 */
static int match(int namelen,const char * name,struct ext2_dir_entry_2 * de)
{	
	if(de->name_len != namelen)
		return -1;
	int len=de->name_len-1;
	for(;len>=0;len--)
		if(get_fs_byte(name+len)!=de->name[len])
			return -1;
	return 0;
}


/*
 *	find_entry()
 *
 * finds an entry in the specified directory with the wanted name. It
 * returns the cache buffer in which the entry was found, and the entry
 * itself (as a parameter - res_dir). It does NOT read the inode of the
 * entry - you'll have to do that yourself if you want to.
 *
 * This also takes care of the few special cases due to '..'-traversal
 * over a pseudo-root and a mount point.
 */
 //�����dir����Ϊstruct m_inode **��ԭ���ǣ��ڷ�root���ص���Ѱ"..",��ʹĿ¼�л���
static buffer_head * find_entry(m_inode ** dir,
	const char * name, int namelen, ext2_dir_entry_2 ** res_dir,ext2_dir_entry_2 **last_dir)
{
	int block,i,size;
	buffer_head * bh;
	ext2_dir_entry_2 * de;
	m_super_block * sb;
	
	*res_dir = NULL;
	if (!namelen)
		return NULL;
	

/* check for '..', as we might have to do some "magic" for it */
	if (namelen==2 && get_fs_byte(name)=='.' && get_fs_byte(name+1)=='.') {
/* '..' in a pseudo-root results in a faked '.' (just change namelen) */
		if ((*dir) == current->root)
			namelen=1;
		else if ((*dir)->i_num == ROOT_INO) {
/* '..' over a mount-point results in 'dir' being exchanged for the mounted
   directory-inode. NOTE! We set mounted, so that we can iput the new dir */
			sb=get_super((*dir)->i_dev);
			if (sb->s_imount) {
				iput(*dir);
				(*dir)=sb->s_imount;
				(*dir)->i_count++;
			}
		}
	}
	
	size=(*dir)->i_size;
	for(i=0; size>0; i++,size-=BLOCK_SIZE){
		if(!(block=bmap(*dir,i++)))//�ļ��ն�
			continue;
		if(!(bh=bread((*dir)->i_dev,block)))
			continue;
		de = (ext2_dir_entry_2 *) bh->b_data;
		
		while(1){
			if(!match(namelen,name,de))//���ƥ�䣬�򷵻غ�Ŀ¼��Ļ���ҳ
			{
				*res_dir=de;
				if(last_dir){//��һ��Ŀ¼���Ҫ����ɾ������
					*last_dir=NULL;//��������ع�����д
				}
				return bh;
			}
			de=(ext2_dir_entry_2*)((unsigned int)de+de->rec_len);
			/*	���Ŀ¼��Ϊ�� ���� ��ҳ�Ѽ����꣬�����������һҳĿ¼��	*/
			if( (!de->rec_len) || \
				(unsigned int )de+de->rec_len > (unsigned int)bh->b_data+BLOCK_SIZE )
				break;
		}
		brelse(bh);
	}
	return NULL;
}

	/*
 *	get_dir()
 *
 * Getdir traverses the pathname until it hits the topmost directory.
 * It returns NULL on failure.
 */
//static 
m_inode * get_dir(const char * pathname)
{
	char c;
	const char * thisname;
	m_inode * inode;
	buffer_head * bh;
	int namelen,inr,idev;
	ext2_dir_entry_2 * de;

	if (!current->root || !current->root->i_count)
		panic("No root inode");
	if (!current->pwd || !current->pwd->i_count)
		panic("No cwd inode");
	if ((c=get_fs_byte(pathname))=='/') {
		inode = current->root;
		pathname++;
	} else if (c)
		inode = current->pwd;
	else
		return NULL;	/* empty name is bad */
	inode->i_count++;
	while (1) {
		thisname = pathname;
		/*if (!S_ISDIR(inode->i_mode) || !permission(inode,MAY_EXEC)) {
			iput(inode);
			return NULL;
		}*/
		for(namelen=0;(c=get_fs_byte(pathname++))&&(c!='/');namelen++)
			/* nothing */ ;
		if (!c)
			return inode;
		if (!(bh = find_entry(&inode,thisname,namelen,&de,NULL))) {
			iput(inode);
			return NULL;
		}
		inr = de->inode;
		idev = inode->i_dev;
		brelse(bh);
		iput(inode);
		if (!(inode = iget(idev,inr)))
			return NULL;
	}
}
/*
 *	dir_namei()
 *
 * dir_namei() returns the inode of the directory of the
 * specified name, and the name within that directory.
 */
static m_inode * dir_namei(const char * pathname,
	int * namelen, const char ** name)
{
	char c;
	const char* filename;
	m_inode * dir;
	if (!(dir = get_dir(pathname)))
		return NULL;
	
	filename=pathname;
	for(;c=get_fs_byte(pathname++);)
		if(c=='/')
			filename=pathname;
			
	*name = filename;
	*namelen = pathname-filename-1;
	return dir;
}

/*
 *	namei()
 *
 * is used by most simple commands to get the inode of a specified name.
 * Open, link etc use their own routines, but this is enough for things
 * like 'chmod' etc.
 */
m_inode * namei(const char * pathname)
{
	const char * basename;
	int inr,dev,namelen;
	m_inode * dir;
	buffer_head * bh;
	ext2_dir_entry_2 * de;

	if (!(dir = dir_namei(pathname,&namelen,&basename)))
		return NULL;
	if (!namelen)			/* special case: '/usr/' etc */
		return dir;
	bh = find_entry(&dir,basename,namelen,&de,NULL);
	if (!bh) {
		iput(dir);
		return NULL;
	}
	inr = de->inode;
	dev = dir->i_dev;
	brelse(bh);
	iput(dir);
	dir=iget(dev,inr);
	if (dir) {
		dir->i_atime=CURRENT_TIME;
		dir->i_dirt=1;
	}
	return dir;
}



/*
 *	add_entry()
 *
 * adds a file entry to the specified directory, using the same
 * semantics as find_entry(). It returns NULL if it failed.
 *
 * NOTE!! The inode part of 'de' is left at 0 - which means you
 * may not sleep between calling this and putting something into
 * the entry, as someone else might have used it while you slept.
 */

static buffer_head * add_entry(m_inode * dir,
	const char * name, int namelen,ext2_dir_entry_2 ** res_dir)
{
	int block,i,j;
	buffer_head * bh;
	ext2_dir_entry_2 * de;
	m_inode *inode;
	if(!dir || !name || !namelen )
		return NULL;

	for(i=0; ;i++){
		if(!(block=create_block(dir,i++)))//�ļ��ն�
			continue;
		if(!(bh=bread(dir->i_dev,block)))
			continue;
		de = (ext2_dir_entry_2 *) bh->b_data;
		
		while(1){
			//ʣ��ռ䲻�������µ�Ŀ¼��
			if((u32)de+8+(namelen-1)/4*4+1 >(u32)bh->b_data +BLOCK_SIZE){
				brelse(bh);
				break;
			}
			if(de->inode){//��ռ��Ŀ¼��
				de=(ext2_dir_entry_2*)((u32)de+de->rec_len);
				continue;
			}
			
			de->name_len=namelen;
			de->rec_len=8+(namelen-1)/4*4+1;
			/*de->file_type=���ô˺����������ļ�����,
			de->inodeҲ�ɵ��������,
			��ˣ����Ա������ͨ�ļ���Ŀ¼�ļ��Ⱥ�������*/
			for(j=0;j<namelen;j++){
				de->name[j]=get_fs_byte(name++);
			}
			bh->b_dirt=1;

			dir->i_atime=CURRENT_TIME;
			if(dir->i_size<(i+1)*BLOCK_SIZE){//Ŀ¼�ļ���������
				dir->i_size=(i+1)*BLOCK_SIZE;
				dir->i_ctime=CURRENT_TIME;
			}
			dir->i_dirt=1;
			if(res_dir)
				*res_dir=de;
			
			return bh;
		}
	}		
}


int sys_mknod(const char * filename, int mode, int dev)
{
	const char * basename;
	int namelen;
	m_inode * dir, * inode;
	buffer_head * bh;
	ext2_dir_entry_2 * de;
	
	if (!suser())
		return -EPERM;
	if (!(dir = dir_namei(filename,&namelen,&basename)))
		return -ENOENT;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
	}
	if(bh = find_entry(&dir,basename,namelen,&de,NULL)){
		brelse(bh);
		iput(dir);
		return -EEXIST;
	}
	if (!(inode= new_inode(dir->i_dev))) {
		iput(dir);
		return -ENOSPC;
	}
	if (!(bh= add_entry(dir,basename,namelen,&de))){
		iput(dir);
		free_inode(inode);
		iput(inode);
		return -ENOSPC;
	}
	if (S_ISBLK(mode) || S_ISCHR(mode))
		inode->i_zone[0] = dev;
	

	inode->i_mode = mode;
	inode->i_mtime = inode->i_atime = CURRENT_TIME;
	inode->i_dirt = 1;
	
	de->inode = inode->i_num;
	if(S_ISBLK(mode)){
		de->file_type=EXT2_ENTRY_BLK;//���豸
	}else if(S_ISCHR(mode)){
		de->file_type=EXT2_ENTRY_CHR;//�ַ��豸
	}else if(S_ISFIFO(mode)){
		de->file_type=EXT2_ENTRY_FIFO;//�ܵ�
	}else if(S_ISREG(mode)){
		de->file_type=EXT2_ENTRY_REG;//��ͨ�ļ�
	}else{
		panic("unknow file type");
	}	
	bh->b_dirt = 1;
	
	iput(dir);
	iput(inode);
	brelse(bh);
	return 0;
}

int sys_mkdir(const char * pathname, int mode)
{
	const char * basename;
	int namelen;
	m_inode * dir, * inode;
	buffer_head * bh, *dir_block;
	ext2_dir_entry_2 * de;

	if (!suser())
		return -EPERM;
	if (!(dir = dir_namei(pathname,&namelen,&basename)))
		return -ENOENT;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
	}
	if(bh = find_entry(&dir,basename,namelen,&de,NULL)){
		brelse(bh);
		iput(dir);
		return -EEXIST;
	}
	if (!(inode=new_inode(dir->i_dev))) {
		iput(dir);
		return -ENOSPC;
	}
	if(!(dir_block=new_block(inode->i_dev))){
		iput(dir);
		free(inode);	//�½��Ϊ��,����������������ϵ�����.
		iput(inode);
		return -ENOSPC;
	}

	//���"."Ŀ¼��
	de=(ext2_dir_entry_2*)dir_block->b_data;
	de->inode=inode->i_num;
	de->file_type=EXT2_ENTRY_DIR;
	de->name_len=1;
	de->rec_len=12;
	de->name[0]='.';
	inode->i_links_count++;

	//���".."Ŀ¼��
	de=(ext2_dir_entry_2*)((u32)de+de->rec_len);
	de->inode=dir->i_num;
	de->file_type=EXT2_ENTRY_DIR;
	de->name_len=2;
	de->rec_len=12;
	de->name[0]='.';
	de->name[1]='.';
	dir->i_links_count++;
	dir_block->b_dirt=1;

	//���ø�Ŀ¼��Ŀ¼��
	bh=add_entry(dir,basename,namelen,&de);
	de->file_type=EXT2_ENTRY_DIR;
	de->inode=inode->i_num;
	bh->b_dirt=1;
	brelse(bh);

	//����Ҫ��ӵ�Ŀ¼��������
	inode->i_zone[0]=dir_block->b_blocknr;
	inode->i_size=BLOCK_SIZE;
	inode->i_blocks=1;
	inode->i_mtime=inode->i_atime=inode->i_ctime=CURRENT_TIME;
	inode->i_mode=S_IFDIR | (mode & 0777 & ~current->umask);
	inode->i_dirt=1;

	iput(dir);
	iput(inode);
	brelse(dir_block);
	return 0;

}

/*
 * routine to check that the specified directory is empty (for rmdir)
 */
static int empty_dir(m_inode * dir)
{
	int nr,block;
	int len,size,i;
	buffer_head * bh;
	ext2_dir_entry_2 * de;

	for(nr=0;nr*BLOCK_SIZE<dir->i_size;i++){
		if(!(block=bmap(dir,i++)))//�ļ��ն�
			continue;
		if(!(bh=bread(dir->i_dev,block)))
			continue;
		de = (ext2_dir_entry_2 *) bh->b_data;
		
		while(1){
			if( de->name[0]=='.' && (de->name_len==1||de->name[1]=='.'&&de->name_len==2) ){
				de=(ext2_dir_entry_2 *)((u32)de+de->rec_len);
				if((u32)de>=(u32)bh->b_data +BLOCK_SIZE || !de->inode)
					break;
				continue;
			}
			brelse(bh);
			//�ҵ���"."��".."��Ŀ¼��
			return 0;
		}
	}
	brelse(bh);
	//��Ŀ¼
	return 1;
	
}

int sys_rmdir(const char * name)
{
	const char * basename;
	int namelen;
	m_inode * dir, * inode;
	buffer_head * bh;
	ext2_dir_entry_2 * de,*last_de;

	if (!suser())
		return -EPERM;
	if (!(dir = dir_namei(name,&namelen,&basename)))
		return -ENOENT;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
	}
	bh = find_entry(&dir,basename,namelen,&de,&last_de);
	if (!bh) {
		iput(dir);
		return -ENOENT;
	}
	if (!(inode = iget(dir->i_dev, de->inode))) {
		iput(dir);
		brelse(bh);
		return -EPERM;
	}
	if ((dir->i_mode & S_ISVTX) && current->euid &&
	    inode->i_uid != current->euid) {
		iput(dir);
		iput(inode);
		brelse(bh);
		return -EPERM;
	}
	if (inode->i_dev != dir->i_dev || inode->i_count>1) {
		iput(dir);
		iput(inode);
		brelse(bh);
		return -EPERM;
	}
	if (inode == dir) {	/* we may not delete ".", but "../dir" is ok */
		iput(inode);
		iput(dir);
		brelse(bh);
		return -EPERM;
	}
	if (!S_ISDIR(inode->i_mode)) {
		iput(inode);
		iput(dir);
		brelse(bh);
		return -ENOTDIR;
	}
	if (!empty_dir(inode)) {
		iput(inode);
		iput(dir);
		brelse(bh);
		return -ENOTEMPTY;
	}
	if (inode->i_links_count != 2)
		printk("empty directory has nlink!=2 (%d)",inode->i_links_count);
	//��ɾ��Ŀ¼
	last_de->rec_len+=de->rec_len;
	bh->b_dirt = 1;
	brelse(bh);
	inode->i_links_count=0;
	inode->i_dtime=CURRENT_TIME;
	inode->i_dirt=1;
	truncate(inode);
	free_inode(inode);
	
	//��Ŀ¼
	dir->i_links_count--;
	dir->i_ctime = dir->i_mtime = CURRENT_TIME;
	dir->i_dirt=1;
	iput(dir);
	iput(inode);
	return 0;
}

int sys_unlink(const char * name)
{
	const char * basename;
	int namelen;
	m_inode * dir, * inode;
	buffer_head * bh;
	ext2_dir_entry_2 * de,*last_de;

	if (!(dir = dir_namei(name,&namelen,&basename)))
		return -ENOENT;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
	}
	bh = find_entry(&dir,basename,namelen,&de,&last_de);
	if (!bh) {
		iput(dir);
		return -ENOENT;
	}
	if (!(inode = iget(dir->i_dev, de->inode))) {
		iput(dir);
		brelse(bh);
		return -ENOENT;
	}
	if ((dir->i_mode & S_ISVTX) && !suser() &&
	    current->euid != inode->i_uid &&
	    current->euid != dir->i_uid) {
		iput(dir);
		iput(inode);
		brelse(bh);
		return -EPERM;
	}
	if (S_ISDIR(inode->i_mode)) {
		iput(inode);
		iput(dir);
		brelse(bh);
		return -EPERM;
	}
	if (!inode->i_links_count) {
		printk("Deleting nonexistent file (%04x:%d), %d\n",
			inode->i_dev,inode->i_num,inode->i_links_count);
		inode->i_links_count=1;
	}
	
	last_de->rec_len+=de->rec_len;//ǰһ��Ŀ¼������ӣ���������Ŀ¼��
	bh->b_dirt = 1;
	brelse(bh);
	inode->i_links_count--;
	inode->i_dirt = 1;
	inode->i_ctime = CURRENT_TIME;
	/* ���i �ڵ��������Ϊ0�����ͷŸ�i �ڵ�������߼��飬���ͷŸ�i �ڵ㡣
	�˽��źͽ����ռ�õ�block�Ŷ�Ӧ��bitmap�������Ϊδʹ�ã�
	����Ȼ��������Ϣ����block���ݣ��˽��������Ȼ���Ա���ȡ��ֱ��������*/
	if (!inode->i_links_count) {
		truncate(inode);
		free_inode(inode);
	}
	iput(inode);
	iput(dir);
	return 0;
}

int sys_link(const char * oldname, const char * newname)
{
	ext2_dir_entry_2 * de;
	m_inode * oldinode, * dir;
	buffer_head * bh;
	const char * basename;
	int namelen;

	oldinode=namei(oldname);
	if (!oldinode)
		return -ENOENT;
	if (S_ISDIR(oldinode->i_mode)) {//�����Ը�Ŀ¼����������
		iput(oldinode);
		return -EPERM;
	}
	dir = dir_namei(newname,&namelen,&basename);
	if (!dir) {
		iput(oldinode);
		return -EACCES;
	}
	if (!namelen) {
		iput(oldinode);
		iput(dir);
		return -EPERM;
	}
	if (dir->i_dev != oldinode->i_dev) {//����ͬһ�豸
		iput(dir);
		iput(oldinode);
		return -EXDEV;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		iput(oldinode);
		return -EACCES;
	}
	bh = find_entry(&dir,basename,namelen,&de,NULL);//�Ƿ�����ͬ���ļ�
	if (bh) {
		brelse(bh);
		iput(dir);
		iput(oldinode);
		return -EEXIST;
	}
	bh = add_entry(dir,basename,namelen,&de);
	if (!bh) {
		iput(dir);
		iput(oldinode);
		return -ENOSPC;
	}
	de->inode = oldinode->i_num;
	bh->b_dirt = 1;
	brelse(bh);
	iput(dir);
	oldinode->i_links_count++;
	oldinode->i_ctime = CURRENT_TIME;
	oldinode->i_dirt = 1;
	iput(oldinode);
	return 0;
}

/*
 *	open_namei()
 *
 * namei for open - this is in fact almost the whole open-routine.
 */
int open_namei(const char * pathname, int flag, int mode,m_inode ** res_inode)
{
	const char * basename;
	int inr,dev,namelen;
	m_inode * dir, *inode;
	buffer_head * bh;
	ext2_dir_entry_2 * de;
	if ((flag & O_TRUNC) && !(flag & O_ACCMODE))
		flag |= O_WRONLY;
	mode &= 0777 & ~current->umask;	//�½��ļ�Ȩ��
	mode |= S_IFREG;	//Ĭ�ϴ�����ͨ�ļ�
	if (!(dir = dir_namei(pathname,&namelen,&basename)))
		return -ENOENT;
	if (!namelen) {			/* special case: '/usr/' etc */
		if (!(flag & (O_ACCMODE|O_CREAT|O_TRUNC))) {
			*res_inode=dir;
			return 0;
		}
		iput(dir);
		return -EISDIR;
	}
	bh = find_entry(&dir,basename,namelen,&de,NULL);
	
	if (!bh) {	//�����ڶ�Ӧ�ļ�
		if (!(flag & O_CREAT)) {
			iput(dir);
			return -ENOENT;
		}
		if (!permission(dir,MAY_WRITE)) {
			iput(dir);
			return -EACCES;
		}
		
		if(!(inode=new_inode(dir->i_dev))){
			iput(dir);
			return -ENOSPC;
		}

		if(!(bh=add_entry(dir,basename,namelen,&de))){
			free_inode(inode);
			iput(dir);
			return -ENOSPC;
		}	
		
		inode->i_mode = mode;	//�������
		de->inode=inode->i_num;
		de->file_type=EXT2_ENTRY_REG;//Ŀ¼������:��ͨ�ļ�
		
		bh->b_dirt = 1;
		inode->i_dirt = 1;
		brelse(bh);
		iput(dir);
		*res_inode = inode;
		return 0;
	}
	
	inr = de->inode;
	dev = dir->i_dev;
	brelse(bh);
	iput(dir);
	
	if (flag & O_EXCL)
		return -EEXIST;
	if (!(inode=iget(dev,inr)))
		return -EACCES;
	//��֤��������(flag)���ļ�Ȩ��(mode)�Ƿ�Ϸ�,ע:Ŀ¼�ļ�����д,��ʹ��дȨ��
	if ((S_ISDIR(inode->i_mode) && (flag & O_ACCMODE)) ||
	    !permission(inode,ACC_MODE(flag))) {
		iput(inode);
		return -EPERM;
	}
	inode->i_atime = CURRENT_TIME;
	if (flag & O_TRUNC)
		panic("not support truncate!(open_namei())");
		//truncate(inode);
	*res_inode = inode;
	return 0;
}


