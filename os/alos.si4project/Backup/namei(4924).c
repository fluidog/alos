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
int permission(m_inode * inode,int mask)
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
 //传入的dir类型为struct m_inode **的原因是：在非root挂载点搜寻"..",会使目录切换。
static buffer_head * find_entry(m_inode ** dir,
	const char * name, int namelen, ext2_dir_entry_2 ** res_dir,ext2_dir_entry_2 **last_dir)
{
	int block,i,size;
	buffer_head * bh;
	ext2_dir_entry_2 * de,*last;
	m_super_block * sb;
	
	
	if (!namelen || !dir ||!name ||!res_dir)
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

	for(i=0;;i++){
		if(((i+1)*BLOCK_SIZE)>(*dir)->i_size)//全部检索完
			break;
		if(!(block=bmap(*dir,i++)))//文件空洞
			continue;
		if(!(bh=bread((*dir)->i_dev,block)))
			continue;
		de = (ext2_dir_entry_2 *) bh->b_data;
		last=NULL;
		while(1){
			if(namelen==de->name_len && !match(namelen,name,de))//如果匹配，则返回含目录项的缓存页
			{
				*res_dir=de;
				if(last_dir){//上一个目录项，主要用于删除操作
					*last_dir=last;//待会儿，重构代码写
				}
				return bh;
			}
			last=de;
			de=(ext2_dir_entry_2*)((unsigned int)de+de->rec_len);
			/*	如果目录项为空 或者 此页已检索完，则继续读入下一页目录项	*/
			if( (!de->rec_len) || \
				(char *)de >= bh->b_data+BLOCK_SIZE )
				break;
		}
		brelse(bh);
	}
	return NULL;
}

/*
*	dir_namei()
*
* dir_namei() returns the inode of the directory of the
* specified name, and the name within that directory.
*/

static m_inode * dir_namei(const char * pathname,int * namelen, const char ** name)
{
	char c;
	const char * thisname;
	m_inode * inode;
	buffer_head * bh;
	int len,inr,idev;
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
		if (!S_ISDIR(inode->i_mode)||!permission(inode,MAY_EXEC) ) {
			iput(inode);
			return NULL;
		}
	
		for(len=0;;len++){
			c=get_fs_byte(pathname++);
			if(c=='/')
				break;
			if(!c){
				*namelen=len;
				if(len)
					*name=thisname;
				else	//有可能给出的路径没有文件名,即/etc/
					*name=NULL;
				return inode;
			}
		}
		if (!(bh = find_entry(&inode,thisname,len,&de,NULL))) {
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
	int block,i,j,used_len,need_len;
	buffer_head * bh;
	ext2_dir_entry_2 * de,*new_de;
	m_inode *inode;
	if(!dir || !name || !namelen )
		return NULL;

	need_len=8+(namelen-1)/4*4+4;//新目录项所需空间
	for(i=0;;i++){
		if(!(block=create_block(dir,i++)))//文件空洞
			continue;
		if(!(bh=bread(dir->i_dev,block)))
			continue;
		de = (ext2_dir_entry_2 *) bh->b_data;
		
		while(1){
			if(!de->rec_len){//添加"."首个目录项
				new_de=de;
				new_de->name_len=namelen;
				new_de->rec_len=BLOCK_SIZE;
				
			}else{
				used_len=8+(de->name_len-1)/4*4+4;//当前目录项实际占用空间
				if(de->rec_len-used_len<need_len){	//剩余空间不足
					de=(ext2_dir_entry_2 *)((u32)de+de->rec_len);
					if((char *)de>=bh->b_data+BLOCK_SIZE)//读下一页目录
						break;
					continue ;
				}
				//填充新目录项
				new_de=(ext2_dir_entry_2 *)((u32)de +used_len);
				new_de->rec_len=de->rec_len-used_len;
				new_de->name_len=namelen;
				//同步旧目录项
				de->rec_len=used_len;
			}
				
			/*de->file_type=调用此函数者设置文件属性,
			de->inode也由调用者填充,
			因此，可以被添加任何文件的函数调用*/
			for(j=0;j<namelen;j++){
				new_de->name[j]=get_fs_byte(name++);
			}
			bh->b_dirt=1;

			dir->i_atime=CURRENT_TIME;
			if(dir->i_size<(i+1)*BLOCK_SIZE){//目录文件长度增加
				dir->i_size=(i+1)*BLOCK_SIZE;
				dir->i_ctime=CURRENT_TIME;
			}
			dir->i_dirt=1;
			if(res_dir)
				*res_dir=new_de;			
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
		de->file_type=EXT2_ENTRY_BLK;//块设备
	}else if(S_ISCHR(mode)){
		de->file_type=EXT2_ENTRY_CHR;//字符设备
	}else if(S_ISFIFO(mode)){
		de->file_type=EXT2_ENTRY_FIFO;//管道
	}else if(S_ISREG(mode)){
		de->file_type=EXT2_ENTRY_REG;//普通文件
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
		free(inode);	//新结点为空,会清除结点物理介质上的内容.
		iput(inode);
		return -ENOSPC;
	}
	
	DEBUG("mkdir %s inode_num:%d block:%d",pathname,inode->i_num,dir_block->b_blocknr);

	//添加"."目录项
	de=(ext2_dir_entry_2*)dir_block->b_data;
	de->inode=inode->i_num;
	de->file_type=EXT2_ENTRY_DIR;
	de->name_len=1;
	de->rec_len=12;
	de->name[0]='.';
	inode->i_links_count++;
	

	//添加".."目录项
	de=(ext2_dir_entry_2*)((u32)de+de->rec_len);
	de->inode=dir->i_num;
	de->file_type=EXT2_ENTRY_DIR;
	de->name_len=2;
	de->rec_len=BLOCK_SIZE-12;
	de->name[0]='.';
	de->name[1]='.';
	dir->i_links_count++;
	dir_block->b_dirt=1;


	//设置父目录的目录项
	bh=add_entry(dir,basename,namelen,&de);
	de->file_type=EXT2_ENTRY_DIR;
	de->inode=inode->i_num;
	bh->b_dirt=1;
	brelse(bh);

	//设置要添加的目录结点的内容
	inode->i_zone[0]=dir_block->b_blocknr;
	inode->i_size=BLOCK_SIZE;
	inode->i_blocks=2;
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

	for(nr=0;nr*BLOCK_SIZE<dir->i_size;nr++){
		if(!(block=bmap(dir,nr)))//文件空洞
			continue;
		if(!(bh=bread(dir->i_dev,block)))
			continue;
		de = (ext2_dir_entry_2 *) bh->b_data;
		
		while(1){
			if( de->name[0]=='.' && (de->name_len==1||de->name[1]=='.'&&de->name_len==2) ){
				de=(ext2_dir_entry_2 *)((u32)de+de->rec_len);
				if((char *)de>=bh->b_data +BLOCK_SIZE || !de->rec_len)
					break;
				continue;
			}
			brelse(bh);
			//找到除"."和".."的目录项
			return 0;
		}
	}
	brelse(bh);
	//空目录
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
	
	//被删除目录
	last_de->rec_len+=de->rec_len;
	bh->b_dirt = 1;
	brelse(bh);
	inode->i_links_count=0;
	inode->i_dtime=CURRENT_TIME;
	inode->i_dirt=1;
	truncate(inode);
	free_inode(inode);
	
	//父目录
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
	
	last_de->rec_len+=de->rec_len;//前一项目录项长度增加，以跳过此目录项
	bh->b_dirt = 1;
	brelse(bh);
	inode->i_links_count--;
	inode->i_dirt = 1;
	inode->i_ctime = CURRENT_TIME;
	/* 如果i 节点的链接数为0，则释放该i 节点的所有逻辑块，并释放该i 节点。
	此结点号和结点所占用的block号对应的bitmap都被标记为未使用，
	但依然保存结点信息，和block内容，此结点内容依然可以被读取，直到被覆盖*/
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
	if (S_ISDIR(oldinode->i_mode)) {//不可以给目录创建软连接
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
	if (dir->i_dev != oldinode->i_dev) {//必须同一设备
		iput(dir);
		iput(oldinode);
		return -EXDEV;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		iput(oldinode);
		return -EACCES;
	}
	bh = find_entry(&dir,basename,namelen,&de,NULL);//是否已有同名文件
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
	mode &= 0777 & ~current->umask;	//新建文件权限
	mode |= S_IFREG;	//默认创建普通文件
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
	
	if (!bh) {	//不存在对应文件
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
		
		inode->i_mode = mode;	//结点属性
		de->inode=inode->i_num;
		de->file_type=EXT2_ENTRY_REG;//目录项属性:普通文件
		
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
	//验证访问请求(flag)和文件权限(mode)是否合法,注:目录文件不可写,即使有写权限
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


