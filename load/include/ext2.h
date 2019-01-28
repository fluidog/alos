#include<types.h>
#define SUPER_MAGIC 0xEF53	// �ļ�ϵͳħ����(ext2)
#define BLOCK_SIZE 1024
typedef struct d_super_block
{
		__le32	   s_inodes_count;			  /* �����ڵ������ */
		 __le32 	s_blocks_count; 			/* �����������еĿ飩 */
		 __le32 	s_r_blocks_count;		   /* �����Ŀ��� */
		 __le32 	s_free_blocks_count;	 /* ���п��� */
		 __le32 	s_free_inodes_count;	 /* ���������ڵ��� */
		 __le32 	s_first_data_block; 		/* ��һ��ʹ�õĿ�ţ���Ϊ1�� */
		 __le32 	s_log_block_size;			/* ��Ĵ�С */
		 __le32 	s_log_frag_size;			 /* Ƭ�Ĵ�С */
		 __le32 	s_blocks_per_group; 	 /* ÿ���еĿ��� */
		 __le32 	s_frags_per_group;		  /* ÿ���е�Ƭ�� */
		 __le32 	s_inodes_per_group; 	/* ÿ���е������ڵ��� */
		 __le32 	s_mtime;						/* ���һ�ΰ�װ������ʱ�� */
		 __le32 	s_wtime;						/* ���һ��д������ʱ�� */
		 __le16 	s_mnt_count;				 /* ��ִ�а�װ�����Ĵ��� */
		 __le16 	s_max_mnt_count;		/* ���֮ǰ��װ�����Ĵ��� */
		 __le16 	s_magic;				   /* ħ��ǩ�� */
		 __le16 	s_state;					/* �ļ�ϵͳ״̬��־ */
		 __le16 	s_errors;					/* ����⵽����ʱ����Ϊ */
		 __le16 	s_minor_rev_level;		  /* �ΰ汾�� */
		 __le32 	s_lastcheck;				  /* ���һ�μ���ʱ�� */
		 __le32 	s_checkinterval;			/* ���μ��֮���ʱ���� */
		 __le32 	s_creator_os;				 /* �����ļ�ϵͳ�Ĳ���ϵͳ */
		 __le32 	s_rev_level;			 /* ���汾�� */
		 __le16 	s_def_resuid;		   /* �������ȱʡUID */
		 __le16 	s_def_resgid;		   /* �������ȱʡ�û���ID */
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
		 __le32 	s_first_ino;		   /* ��һ���Ǳ����������ڵ�� */
		 __le16   s_inode_size; 		  /* �����������ڵ�ṹ�Ĵ�С */
		 __le16 	s_block_group_nr;	   /* ���������Ŀ���� */
		 __le32 	s_feature_compat;	   /* ���м����ص��λͼ */
		 __le32 	s_feature_incompat; 	 /* ���зǼ����ص��λͼ */
		 __le32 	s_feature_ro_compat;	  /* ֻ�������ص��λͼ */
		 __u8	  s_uuid[16];		   /* 128λ�ļ�ϵͳ��ʶ�� */
		 char	  s_volume_name[16];	  /* ���� */
		 char	  s_last_mounted[64];	   /* ���һ����װ���·���� */
		 __le32 	s_algorithm_usage_bitmap; /* ����ѹ�� */
		 /*
		  * Performance hints.	Directory preallocation should only
		  * happen if the EXT2_COMPAT_PREALLOC flag is on.
		  */
		 __u8	  s_prealloc_blocks;	 /* Ԥ����Ŀ��� */
		 __u8	  s_prealloc_dir_blocks;	 /* ΪĿ¼Ԥ����Ŀ��� */
		 __u16	   s_padding1;			   /* ���ֶ��� */
		// __u32	 s_reserved[190];	  /* ��null���1024�ֽ� */
}d_super_block;


typedef struct group_desc
{
     __le32     bg_block_bitmap;             /* ��λͼ�Ŀ�� */
     __le32     bg_inode_bitmap;            /* �����ڵ�λͼ�Ŀ�� */
     __le32     bg_inode_table;               /* ��һ�������ڵ���Ŀ�� */
     __le16     bg_free_blocks_count;     /* ���п��п�ĸ��� */
     __le16     bg_free_inodes_count;     /* ���п��������ڵ�ĸ��� */
     __le16     bg_used_dirs_count;        /* ����Ŀ¼�ĸ��� */
     __le16     bg_pad;                            /* ���ֶ��� */
     __le32     bg_reserved[3];                /* ��null���24���ֽ� */
}group_desc;
#define     EXT2_NDIR_BLOCKS          12
#define     EXT2_IND_BLOCK               EXT2_NDIR_BLOCKS
#define     EXT2_DIND_BLOCK               (EXT2_IND_BLOCK + 1)
#define     EXT2_TIND_BLOCK               (EXT2_DIND_BLOCK + 1)
#define     EXT2_N_BLOCKS               (EXT2_TIND_BLOCK + 1)

typedef struct d_inode {
     __le16     i_mode;          /* �ļ����ͺͷ���Ȩ�� */
     __le16     i_uid;          /* ӵ���߱�ʶ�� */
     __le32     i_size;          /* ���ֽ�Ϊ��λ���ļ����� */
     __le32     i_atime;     /* ���һ�η����ļ���ʱ�� */
     __le32     i_ctime;     /* �����ڵ����ı��ʱ�� */
     __le32     i_mtime;     /* �ļ��������ı��ʱ�� */
     __le32     i_dtime;     /* �ļ�ɾ����ʱ�� */
     __le16     i_gid;          /* �û����ʶ����16λ */
     __le16     i_links_count;     /* Ӳ���Ӽ����� */
     __le32     i_blocks;     /* �ļ������ݿ��� */
     __le32     i_flags;     /* �ļ���־ */
	 __le32		l_i_reserved1;
     __le32     i_zone[EXT2_N_BLOCKS]; /* ָ�����ݿ��ָ�� */
     __le32     i_generation;     /* �ļ��汾���������ļ�ϵͳ�����ļ�ʱ�� */
     __le32     i_file_acl;     /* �ļ����ʿ����б� */
     __le32     i_dir_acl;     /* Ŀ¼���ʿ����б� */
     __le32     i_faddr;     /* Ƭ�ĵ�ַ */
	 __u8		l_i_frag;     /* Fragment number */
	 __u8		l_i_fsize;     /* Fragment size */
	 __u16		i_pad1;
	 __le16		l_i_uid_high;     /* these 2 fields    */
	__le16		l_i_gid_high;     /* were reserved2[0] */
	__u32		l_i_reserved2;
}d_inode;

#define EXT2_NAME_LEN 255
typedef struct ext2_dir_entry_2 {
     __le32     inode;               /* �����ڵ�� */
     __le16     rec_len;          /* Ŀ¼��� */
     __u8     name_len;          /* �ļ������� */
     __u8     file_type;              /* �ļ����� */
     char     name[EXT2_NAME_LEN];     /* �ļ��� */
}ext2_dir_entry_2;

s8 ext2_load_file(void *loadAddr,char *fileName);



