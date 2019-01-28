#include<types.h>
#define SUPER_MAGIC 0xEF53	// 文件系统魔数。(ext2)
#define BLOCK_SIZE 1024
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

#define EXT2_NAME_LEN 255
typedef struct ext2_dir_entry_2 {
     __le32     inode;               /* 索引节点号 */
     __le16     rec_len;          /* 目录项长度 */
     __u8     name_len;          /* 文件名长度 */
     __u8     file_type;              /* 文件类型 */
     char     name[EXT2_NAME_LEN];     /* 文件名 */
}ext2_dir_entry_2;

s8 ext2_load_file(void *loadAddr,char *fileName);



