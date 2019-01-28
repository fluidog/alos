#ifndef _FS_H_
#define _FS_H_
#include<sys/types.h>

#define IS_SEEKABLE(x) ((x)>=1 && (x)<=3)	// �Ƿ��ǿ���Ѱ�Ҷ�λ���豸��

#define READ 0
#define WRITE 1
#define READA 2			/* read-ahead - don't pause */
#define WRITEA 3		/* "write-ahead" - silly, but somewhat useful */


#define MAJOR(a) (((unsigned)(a))>>8)	// ȡ���ֽڣ����豸�ţ���
#define MINOR(a) ((a)&0xff)	// ȡ���ֽڣ����豸�ţ���

#define NAME_LEN 14		// ���ֳ���ֵ��
#define ROOT_INO 2		// ��i �ڵ㡣

#define I_MAP_SLOTS 8		// i �ڵ�λͼ������
#define Z_MAP_SLOTS 8		// �߼��飨���ο飩λͼ������
#define SUPER_MAGIC 0xEF53	// �ļ�ϵͳħ����(ext2)

#define NR_OPEN 20		// ���ļ�����
#define NR_INODE 32
#define NR_FILE 64
#define NR_SUPER 8
#define NR_HASH 307
#define NR_BUFFERS nr_buffers
#define BLOCK_SIZE 1024		// ���ݿ鳤�ȡ�
#define BLOCK_SIZE_BITS 10	// ���ݿ鳤����ռ����λ����
#define GROUPS_PER_BLOCK (BLOCK_SIZE/sizeof(struct group_desc))
// ÿ���߼���ɴ�ŵ�i �ڵ�����
#define INODES_PER_BLOCK ((BLOCK_SIZE)/(sizeof (struct d_inode)))


// �����ϳ�����ṹ������125-132 ����ȫһ����
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

typedef struct m_super_block 
	{
     __le32     s_inodes_count;            /* �����ڵ������ */
     __le32     s_blocks_count;             /* �����������еĿ飩 */
     __le32     s_r_blocks_count;          /* �����Ŀ��� */
     __le32     s_free_blocks_count;     /* ���п��� */
     __le32     s_free_inodes_count;     /* ���������ڵ��� */
     __le32     s_first_data_block;         /* ��һ��ʹ�õĿ�ţ���Ϊ1�� */
     __le32     s_log_block_size;           /* ��Ĵ�С */
     __le32     s_log_frag_size;             /* Ƭ�Ĵ�С */
     __le32     s_blocks_per_group;      /* ÿ���еĿ��� */
     __le32     s_frags_per_group;        /* ÿ���е�Ƭ�� */
     __le32     s_inodes_per_group;     /* ÿ���е������ڵ��� */
     __le32     s_mtime;                        /* ���һ�ΰ�װ������ʱ�� */
     __le32     s_wtime;                        /* ���һ��д������ʱ�� */
     __le16     s_mnt_count;                 /* ��ִ�а�װ�����Ĵ��� */
     __le16     s_max_mnt_count;        /* ���֮ǰ��װ�����Ĵ��� */
     __le16     s_magic;                   /* ħ��ǩ�� */
     __le16     s_state;                    /* �ļ�ϵͳ״̬��־ */
     __le16     s_errors;                   /* ����⵽����ʱ����Ϊ */
     __le16     s_minor_rev_level;        /* �ΰ汾�� */
     __le32     s_lastcheck;                  /* ���һ�μ���ʱ�� */
     __le32     s_checkinterval;            /* ���μ��֮���ʱ���� */
     __le32     s_creator_os;                /* �����ļ�ϵͳ�Ĳ���ϵͳ */
     __le32     s_rev_level;             /* ���汾�� */
     __le16     s_def_resuid;          /* �������ȱʡUID */
     __le16     s_def_resgid;          /* �������ȱʡ�û���ID */
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
     __le32     s_first_ino;           /* ��һ���Ǳ����������ڵ�� */
     __le16   s_inode_size;           /* �����������ڵ�ṹ�Ĵ�С */
     __le16     s_block_group_nr;      /* ���������Ŀ���� */
     __le32     s_feature_compat;      /* ���м����ص��λͼ */
     __le32     s_feature_incompat;      /* ���зǼ����ص��λͼ */
     __le32     s_feature_ro_compat;      /* ֻ�������ص��λͼ */
     __u8     s_uuid[16];          /* 128λ�ļ�ϵͳ��ʶ�� */
     char     s_volume_name[16];      /* ���� */
     char     s_last_mounted[64];      /* ���һ����װ���·���� */
     __le32     s_algorithm_usage_bitmap; /* ����ѹ�� */
     /*
      * Performance hints.  Directory preallocation should only
      * happen if the EXT2_COMPAT_PREALLOC flag is on.
      */
     __u8     s_prealloc_blocks;     /* Ԥ����Ŀ��� */
     __u8     s_prealloc_dir_blocks;     /* ΪĿ¼Ԥ����Ŀ��� */
     __u16     s_padding1;             /* ���ֶ��� */
    // __u32     s_reserved[190];     /* ��null���1024�ֽ� */

	/* These are only in memory */
  struct buffer_head *s_group[8];	// ����ʾ256��
  struct buffer_head *s_imap[256];	// i �ڵ�λͼ�����ָ������(ռ��8 �飬�ɱ�ʾ64M)��
  struct buffer_head *s_zmap[256];	// �߼���λͼ�����ָ�����飨ռ��8 �飩��
  unsigned short s_dev;		// ���������ڵ��豸�š�
  struct m_inode *s_isup;	// ����װ���ļ�ϵͳ��Ŀ¼��i �ڵ㡣(isup-super i)
  struct m_inode *s_imount;	// ����װ����i �ڵ㡣
  unsigned long s_time;		// �޸�ʱ�䡣
  struct task_struct *s_wait;	// �ȴ��ó�����Ľ��̡�
  unsigned char s_lock;		// ��������־��
  unsigned char s_rd_only;	// ֻ����־��
  unsigned char s_dirt;		// ���޸�(��)��־��
}m_super_block;


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

// �������ڴ��е�i �ڵ�ṹ��ǰ7 ����d_inode ��ȫһ����
typedef struct m_inode
{
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
/* these are in memory also */
	struct task_struct *i_wait;	// �ȴ���i �ڵ�Ľ��̡�
	unsigned short i_dev;		// i �ڵ����ڵ��豸�š�
	unsigned short i_num;		// i �ڵ�š�
	unsigned short i_count;	// i �ڵ㱻ʹ�õĴ�����0 ��ʾ��i �ڵ���С�
	unsigned char i_lock;		// ������־��
	unsigned char i_dirt;		// ���޸�(��)��־��
	unsigned char i_pipe;		// �ܵ���־��
	unsigned char i_mount;	// ��װ��־��
	unsigned char i_seek;		// ��Ѱ��־(lseek ʱ)��
	unsigned char i_update;	// ���±�־��
}m_inode;



#define EXT2_NAME_LEN 255
typedef struct ext2_dir_entry_2 {
     __le32     inode;               /* �����ڵ�� */
     __le16     rec_len;          /* Ŀ¼��� */
     __u8     name_len;          /* �ļ������� */
     __u8     file_type;              /* �ļ����� */
     char     name[EXT2_NAME_LEN];     /* �ļ��� */
}ext2_dir_entry_2;


typedef struct buffer_head
{
  char *b_data;			/* pointer to data block (1024 bytes) *///ָ�롣
  unsigned long b_blocknr;	/* block number */// ��š�
  unsigned short b_dev;		/* device (0 = free) */// ����Դ���豸�š�
  unsigned char b_uptodate;	// ���±�־����ʾ�����Ƿ��Ѹ��¡�
  unsigned char b_dirt;		/* 0-clean,1-dirty *///�޸ı�־:0 δ�޸�,1 ���޸�.
  unsigned char b_count;	/* users using this block */// ʹ�õ��û�����
  unsigned char b_lock;		/* 0 - ok, 1 -locked */// �������Ƿ�������
  struct task_struct *b_wait;	// ָ��ȴ��û���������������
  struct buffer_head *b_prev;	// hash ������ǰһ�飨���ĸ�ָ�����ڻ������Ĺ�����
  struct buffer_head *b_next;	// hash ��������һ�顣
  struct buffer_head *b_prev_free;	// ���б���ǰһ�顣
  struct buffer_head *b_next_free;	// ���б�����һ�顣
}buffer_head;

// �ļ��ṹ���������ļ������i �ڵ�֮�佨����ϵ��
typedef struct file
{
  unsigned short f_mode;	// �ļ�����ģʽ��RW λ��
  unsigned short f_flags;	// �ļ��򿪺Ϳ��Ƶı�־��
  unsigned short f_count;	// ��Ӧ�ļ�������ļ�������������
  struct m_inode *f_inode;	// ָ���Ӧi �ڵ㡣
  off_t f_pos;			// �ļ�λ�ã���дƫ��ֵ����
}file;

//ȫ�ֱ���
extern file file_table[NR_FILE];
extern m_super_block super_block[NR_SUPER];
extern m_inode inode_table[NR_INODE];
extern int ROOT_DEV;

/*			������					*/
void buffer_init (long buffer_end);
// ��ȡָ�������ݿ顣
buffer_head *bread (int dev, int block);
buffer_head *breada (int dev, int block, ...);
int sys_sync(void);
int sync_dev(int dev);
// �ͷ�ָ������顣
void brelse (buffer_head *buf);
//��д���ݿ�
void ll_rw_block (int rw,buffer_head *bh);
void dis_play_buffer();


/*		������(super block)		*/
void mount_root(void);
m_super_block * get_super(int dev);
group_desc* get_group(m_super_block *sb,int group);


/*		���(inode)				*/
m_inode * iget(int dev,int nr);
void iput(m_inode * inode);
int bmap(m_inode * inode,int block);
void sync_inodes(void);



/*		�ļ�						*/
m_inode * namei(const char * pathname);
int open_namei(const char * pathname, int flag, int mode,m_inode ** res_inode);
int file_read(file * filp, char * buf, int count);
int block_read(int dev, unsigned long * pos, char * buf, int count);


#endif







