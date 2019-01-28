#ifndef _FS_H_
#define _FS_H_
#include<sys/types.h>

#define IS_SEEKABLE(x) ((x)>=1 && (x)<=3)	// 是否是可以寻找定位的设备。

#define READ 0
#define WRITE 1
#define READA 2			/* read-ahead - don't pause */
#define WRITEA 3		/* "write-ahead" - silly, but somewhat useful */


#define MAJOR(a) (((unsigned)(a))>>8)	// 取高字节（主设备号）。
#define MINOR(a) ((a)&0xff)	// 取低字节（次设备号）。

#define NAME_LEN 14		// 名字长度值。
#define ROOT_INO 2		// 根i 节点。

#define I_MAP_SLOTS 8		// i 节点位图槽数。
#define Z_MAP_SLOTS 8		// 逻辑块（区段块）位图槽数。
#define SUPER_MAGIC 0xEF53	// 文件系统魔数。(ext2)

#define NR_OPEN 20		// 打开文件数。
#define NR_INODE 32
#define NR_FILE 64
#define NR_SUPER 8
#define NR_HASH 307
#define NR_BUFFERS nr_buffers
#define BLOCK_SIZE 1024		// 数据块长度。
#define BLOCK_SIZE_BITS 10	// 数据块长度所占比特位数。
#define GROUPS_PER_BLOCK (BLOCK_SIZE/sizeof(struct group_desc))
// 每个逻辑块可存放的i 节点数。
#define INODES_PER_BLOCK ((BLOCK_SIZE)/(sizeof (struct d_inode)))


// 磁盘上超级块结构。上面125-132 行完全一样。
typedef struct d_super_block
{
		__le32	   s_inodes_count;			  /* 索引节点的总数 */
		 __le32 	s_blocks_count; 			/* 块总数（所有的块） */
		 __le32 	s_r_blocks_count;		   /* 保留的块数 */
		 __le32 	s_free_blocks_count;	 /* 空闲块数 */
		 __le32 	s_free_inodes_count;	 /* 空闲索引节点数 */
		 __le32 	s_first_data_block; 		/* 第一个使用的块号（总为1） */
		 __le32 	s_log_block_size;			/* 块的大小 */
		 __le32 	s_log_frag_size;			 /* 片的大小 */
		 __le32 	s_blocks_per_group; 	 /* 每组中的块数 */
		 __le32 	s_frags_per_group;		  /* 每组中的片数 */
		 __le32 	s_inodes_per_group; 	/* 每组中的索引节点数 */
		 __le32 	s_mtime;						/* 最后一次安装操作的时间 */
		 __le32 	s_wtime;						/* 最后一次写操作的时间 */
		 __le16 	s_mnt_count;				 /* 被执行安装操作的次数 */
		 __le16 	s_max_mnt_count;		/* 检查之前安装操作的次数 */
		 __le16 	s_magic;				   /* 魔术签名 */
		 __le16 	s_state;					/* 文件系统状态标志 */
		 __le16 	s_errors;					/* 当检测到错误时的行为 */
		 __le16 	s_minor_rev_level;		  /* 次版本号 */
		 __le32 	s_lastcheck;				  /* 最后一次检查的时间 */
		 __le32 	s_checkinterval;			/* 两次检查之间的时间间隔 */
		 __le32 	s_creator_os;				 /* 创建文件系统的操作系统 */
		 __le32 	s_rev_level;			 /* 主版本号 */
		 __le16 	s_def_resuid;		   /* 保留块的缺省UID */
		 __le16 	s_def_resgid;		   /* 保留块的缺省用户组ID */
		 /*
		  * These fields are for EXT2_DYNAMIC_REV superblocks only.
		  *
		  * Note: the difference between the compatible feature set and
		  * the incompatible feature set is that if there is a bit set
		  * in the incompatible feature set that the kernel doesn't
		  * know about, it should refuse to mount the filesystem.
		  * 
		  * e2fsck's requirements are more strict; if it doesn't know
		  * about a feature in either the compatible or incompatible
		  * feature set, it must abort and not try to meddle with
		  * things it doesn't understand...
		  */
		 __le32 	s_first_ino;		   /* 第一个非保留的索引节点号 */
		 __le16   s_inode_size; 		  /* 磁盘上索引节点结构的大小 */
		 __le16 	s_block_group_nr;	   /* 这个超级块的块组号 */
		 __le32 	s_feature_compat;	   /* 具有兼容特点的位图 */
		 __le32 	s_feature_incompat; 	 /* 具有非兼容特点的位图 */
		 __le32 	s_feature_ro_compat;	  /* 只读兼容特点的位图 */
		 __u8	  s_uuid[16];		   /* 128位文件系统标识符 */
		 char	  s_volume_name[16];	  /* 卷名 */
		 char	  s_last_mounted[64];	   /* 最后一个安装点的路径名 */
		 __le32 	s_algorithm_usage_bitmap; /* 用于压缩 */
		 /*
		  * Performance hints.	Directory preallocation should only
		  * happen if the EXT2_COMPAT_PREALLOC flag is on.
		  */
		 __u8	  s_prealloc_blocks;	 /* 预分配的块数 */
		 __u8	  s_prealloc_dir_blocks;	 /* 为目录预分配的块数 */
		 __u16	   s_padding1;			   /* 按字对齐 */
		// __u32	 s_reserved[190];	  /* 用null填充1024字节 */
}d_super_block;

typedef struct m_super_block 
	{
     __le32     s_inodes_count;            /* 索引节点的总数 */
     __le32     s_blocks_count;             /* 块总数（所有的块） */
     __le32     s_r_blocks_count;          /* 保留的块数 */
     __le32     s_free_blocks_count;     /* 空闲块数 */
     __le32     s_free_inodes_count;     /* 空闲索引节点数 */
     __le32     s_first_data_block;         /* 第一个使用的块号（总为1） */
     __le32     s_log_block_size;           /* 块的大小 */
     __le32     s_log_frag_size;             /* 片的大小 */
     __le32     s_blocks_per_group;      /* 每组中的块数 */
     __le32     s_frags_per_group;        /* 每组中的片数 */
     __le32     s_inodes_per_group;     /* 每组中的索引节点数 */
     __le32     s_mtime;                        /* 最后一次安装操作的时间 */
     __le32     s_wtime;                        /* 最后一次写操作的时间 */
     __le16     s_mnt_count;                 /* 被执行安装操作的次数 */
     __le16     s_max_mnt_count;        /* 检查之前安装操作的次数 */
     __le16     s_magic;                   /* 魔术签名 */
     __le16     s_state;                    /* 文件系统状态标志 */
     __le16     s_errors;                   /* 当检测到错误时的行为 */
     __le16     s_minor_rev_level;        /* 次版本号 */
     __le32     s_lastcheck;                  /* 最后一次检查的时间 */
     __le32     s_checkinterval;            /* 两次检查之间的时间间隔 */
     __le32     s_creator_os;                /* 创建文件系统的操作系统 */
     __le32     s_rev_level;             /* 主版本号 */
     __le16     s_def_resuid;          /* 保留块的缺省UID */
     __le16     s_def_resgid;          /* 保留块的缺省用户组ID */
     /*
      * These fields are for EXT2_DYNAMIC_REV superblocks only.
      *
      * Note: the difference between the compatible feature set and
      * the incompatible feature set is that if there is a bit set
      * in the incompatible feature set that the kernel doesn't
      * know about, it should refuse to mount the filesystem.
      * 
      * e2fsck's requirements are more strict; if it doesn't know
      * about a feature in either the compatible or incompatible
      * feature set, it must abort and not try to meddle with
      * things it doesn't understand...
      */
     __le32     s_first_ino;           /* 第一个非保留的索引节点号 */
     __le16   s_inode_size;           /* 磁盘上索引节点结构的大小 */
     __le16     s_block_group_nr;      /* 这个超级块的块组号 */
     __le32     s_feature_compat;      /* 具有兼容特点的位图 */
     __le32     s_feature_incompat;      /* 具有非兼容特点的位图 */
     __le32     s_feature_ro_compat;      /* 只读兼容特点的位图 */
     __u8     s_uuid[16];          /* 128位文件系统标识符 */
     char     s_volume_name[16];      /* 卷名 */
     char     s_last_mounted[64];      /* 最后一个安装点的路径名 */
     __le32     s_algorithm_usage_bitmap; /* 用于压缩 */
     /*
      * Performance hints.  Directory preallocation should only
      * happen if the EXT2_COMPAT_PREALLOC flag is on.
      */
     __u8     s_prealloc_blocks;     /* 预分配的块数 */
     __u8     s_prealloc_dir_blocks;     /* 为目录预分配的块数 */
     __u16     s_padding1;             /* 按字对齐 */
    // __u32     s_reserved[190];     /* 用null填充1024字节 */

	/* These are only in memory */
  struct buffer_head *s_group[8];	// 最多表示256组
  struct buffer_head *s_imap[256];	// i 节点位图缓冲块指针数组(占用8 块，可表示64M)。
  struct buffer_head *s_zmap[256];	// 逻辑块位图缓冲块指针数组（占用8 块）。
  unsigned short s_dev;		// 超级块所在的设备号。
  struct m_inode *s_isup;	// 被安装的文件系统根目录的i 节点。(isup-super i)
  struct m_inode *s_imount;	// 被安装到的i 节点。
  unsigned long s_time;		// 修改时间。
  struct task_struct *s_wait;	// 等待该超级块的进程。
  unsigned char s_lock;		// 被锁定标志。
  unsigned char s_rd_only;	// 只读标志。
  unsigned char s_dirt;		// 已修改(脏)标志。
}m_super_block;


typedef struct group_desc
{
     __le32     bg_block_bitmap;             /* 块位图的块号 */
     __le32     bg_inode_bitmap;            /* 索引节点位图的块号 */
     __le32     bg_inode_table;               /* 第一个索引节点表块的块号 */
     __le16     bg_free_blocks_count;     /* 组中空闲块的个数 */
     __le16     bg_free_inodes_count;     /* 组中空闲索引节点的个数 */
     __le16     bg_used_dirs_count;        /* 组中目录的个数 */
     __le16     bg_pad;                            /* 按字对齐 */
     __le32     bg_reserved[3];                /* 用null填充24个字节 */
}group_desc;
#define     EXT2_NDIR_BLOCKS          12
#define     EXT2_IND_BLOCK               EXT2_NDIR_BLOCKS
#define     EXT2_DIND_BLOCK               (EXT2_IND_BLOCK + 1)
#define     EXT2_TIND_BLOCK               (EXT2_DIND_BLOCK + 1)
#define     EXT2_N_BLOCKS               (EXT2_TIND_BLOCK + 1)

typedef struct d_inode {
     __le16     i_mode;          /* 文件类型和访问权限 */
     __le16     i_uid;          /* 拥有者标识符 */
     __le32     i_size;          /* 以字节为单位的文件长度 */
     __le32     i_atime;     /* 最后一次访问文件的时间 */
     __le32     i_ctime;     /* 索引节点最后改变的时间 */
     __le32     i_mtime;     /* 文件内容最后改变的时间 */
     __le32     i_dtime;     /* 文件删除的时间 */
     __le16     i_gid;          /* 用户组标识符低16位 */
     __le16     i_links_count;     /* 硬链接计数器 */
     __le32     i_blocks;     /* 文件的数据块数 */
     __le32     i_flags;     /* 文件标志 */
	 __le32		l_i_reserved1;
     __le32     i_zone[EXT2_N_BLOCKS]; /* 指向数据块的指针 */
     __le32     i_generation;     /* 文件版本（当网络文件系统访问文件时） */
     __le32     i_file_acl;     /* 文件访问控制列表 */
     __le32     i_dir_acl;     /* 目录访问控制列表 */
     __le32     i_faddr;     /* 片的地址 */
	 __u8		l_i_frag;     /* Fragment number */
	 __u8		l_i_fsize;     /* Fragment size */
	 __u16		i_pad1;
	 __le16		l_i_uid_high;     /* these 2 fields    */
	__le16		l_i_gid_high;     /* were reserved2[0] */
	__u32		l_i_reserved2;
}d_inode;

// 这是在内存中的i 节点结构。前7 项与d_inode 完全一样。
typedef struct m_inode
{
   	 __le16     i_mode;          /* 文件类型和访问权限 */
     __le16     i_uid;          /* 拥有者标识符 */
     __le32     i_size;          /* 以字节为单位的文件长度 */
     __le32     i_atime;     /* 最后一次访问文件的时间 */
     __le32     i_ctime;     /* 索引节点最后改变的时间 */
     __le32     i_mtime;     /* 文件内容最后改变的时间 */
     __le32     i_dtime;     /* 文件删除的时间 */
     __le16     i_gid;          /* 用户组标识符低16位 */
     __le16     i_links_count;     /* 硬链接计数器 */
     __le32     i_blocks;     /* 文件的数据块数 */
     __le32     i_flags;     /* 文件标志 */
	 __le32		l_i_reserved1;
     __le32     i_zone[EXT2_N_BLOCKS]; /* 指向数据块的指针 */
     __le32     i_generation;     /* 文件版本（当网络文件系统访问文件时） */
     __le32     i_file_acl;     /* 文件访问控制列表 */
     __le32     i_dir_acl;     /* 目录访问控制列表 */
     __le32     i_faddr;     /* 片的地址 */
	 __u8		l_i_frag;     /* Fragment number */
	 __u8		l_i_fsize;     /* Fragment size */
	 __u16		i_pad1;
	 __le16		l_i_uid_high;     /* these 2 fields    */
	__le16		l_i_gid_high;     /* were reserved2[0] */
	__u32		l_i_reserved2;
/* these are in memory also */
	struct task_struct *i_wait;	// 等待该i 节点的进程。
	unsigned short i_dev;		// i 节点所在的设备号。
	unsigned short i_num;		// i 节点号。
	unsigned short i_count;	// i 节点被使用的次数，0 表示该i 节点空闲。
	unsigned char i_lock;		// 锁定标志。
	unsigned char i_dirt;		// 已修改(脏)标志。
	unsigned char i_pipe;		// 管道标志。
	unsigned char i_mount;	// 安装标志。
	unsigned char i_seek;		// 搜寻标志(lseek 时)。
	unsigned char i_update;	// 更新标志。
}m_inode;



#define EXT2_NAME_LEN 255
typedef struct ext2_dir_entry_2 {
     __le32     inode;               /* 索引节点号 */
     __le16     rec_len;          /* 目录项长度 */
     __u8     name_len;          /* 文件名长度 */
     __u8     file_type;              /* 文件类型 */
     char     name[EXT2_NAME_LEN];     /* 文件名 */
}ext2_dir_entry_2;


typedef struct buffer_head
{
  char *b_data;			/* pointer to data block (1024 bytes) *///指针。
  unsigned long b_blocknr;	/* block number */// 块号。
  unsigned short b_dev;		/* device (0 = free) */// 数据源的设备号。
  unsigned char b_uptodate;	// 更新标志：表示数据是否已更新。
  unsigned char b_dirt;		/* 0-clean,1-dirty *///修改标志:0 未修改,1 已修改.
  unsigned char b_count;	/* users using this block */// 使用的用户数。
  unsigned char b_lock;		/* 0 - ok, 1 -locked */// 缓冲区是否被锁定。
  struct task_struct *b_wait;	// 指向等待该缓冲区解锁的任务。
  struct buffer_head *b_prev;	// hash 队列上前一块（这四个指针用于缓冲区的管理）。
  struct buffer_head *b_next;	// hash 队列上下一块。
  struct buffer_head *b_prev_free;	// 空闲表上前一块。
  struct buffer_head *b_next_free;	// 空闲表上下一块。
}buffer_head;

// 文件结构（用于在文件句柄与i 节点之间建立关系）
typedef struct file
{
  unsigned short f_mode;	// 文件操作模式（RW 位）
  unsigned short f_flags;	// 文件打开和控制的标志。
  unsigned short f_count;	// 对应文件句柄（文件描述符）数。
  struct m_inode *f_inode;	// 指向对应i 节点。
  off_t f_pos;			// 文件位置（读写偏移值）。
}file;

//全局变量
extern file file_table[NR_FILE];
extern m_super_block super_block[NR_SUPER];
extern m_inode inode_table[NR_INODE];
extern int ROOT_DEV;

/*			缓冲区					*/
void buffer_init (long buffer_end);
// 读取指定的数据块。
buffer_head *bread (int dev, int block);
buffer_head *breada (int dev, int block, ...);
int sys_sync(void);
int sync_dev(int dev);
// 释放指定缓冲块。
void brelse (buffer_head *buf);
//读写数据块
void ll_rw_block (int rw,buffer_head *bh);
void dis_play_buffer();


/*		超级块(super block)		*/
void mount_root(void);
m_super_block * get_super(int dev);
group_desc* get_group(m_super_block *sb,int group);


/*		结点(inode)				*/
m_inode * iget(int dev,int nr);
void iput(m_inode * inode);
int bmap(m_inode * inode,int block);
void sync_inodes(void);



/*		文件						*/
m_inode * namei(const char * pathname);
int open_namei(const char * pathname, int flag, int mode,m_inode ** res_inode);
int file_read(file * filp, char * buf, int count);
int block_read(int dev, unsigned long * pos, char * buf, int count);


#endif







