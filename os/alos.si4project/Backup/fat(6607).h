/*
 * R/O (V)FAT 12/16/32 filesystem implementation by Marcus Sundberg
 *
 * 2002-07-28 - rjones@nexus-tech.net - ported to ppcboot v1.1.6
 * 2003-03-10 - kharris@nexus-tech.net - ported to u-boot
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef _FAT_H_
#define _FAT_H_

#include <types.h>


#define SECTORSIZE 512

#define FAT16BUFSIZE SECTORSIZE


#define MAX_CLUSTSIZE	0x10000 //65536
#define DIRENTSPERBLOCK	(FS_BLOCK_SIZE/sizeof(dir_entry))


/* Filesystem identifiers */
#define FAT12_SIGN	"FAT12   "
#define FAT16_SIGN	"FAT16   "
#define FAT32_SIGN	"FAT32   "
#define SIGNLEN		8

/* File attributes */
#define ATTR_RO      1
#define ATTR_HIDDEN  2
#define ATTR_SYS     4
#define ATTR_VOLUME  8
#define ATTR_DIR     16
#define ATTR_ARCH    32

#define ATTR_VFAT     (ATTR_RO | ATTR_HIDDEN | ATTR_SYS | ATTR_VOLUME)

#define DELETED_FLAG	((char)0xe5) /* Marks deleted files when in name[0] */
#define aRING		0x05	     /* Used to represent '? in name[0] */

/* Indicates that the entry is the last long entry in a set of long
 * dir entries
 */
#define LAST_LONG_ENTRY_MASK	0x40

/* Flags telling whether we should read a file or list a directory */
#define LS_NO	0
#define LS_YES	1
#define LS_DIR	1
#define LS_ROOT	2



#define FSTYPE_NONE	(-1)


typedef struct boot_sector {
	__u8	ignored[3];	/* Bootstrap code */
	char	system_id[8];	/* Name of fs */
	__u8	sector_size[2];	/* Bytes/sector */
	__u8	cluster_size;	/* Sectors/cluster */
	__u16	reserved;	/* Number of reserved sectors */
	__u8	fats;		/* Number of FATs */
	__u16	dir_entries;	/* Number of root directory entries */
	__u8	sectors[2];	/* Number of sectors */
	__u8	media;		/* Media code */
	__u16	fat_length;	/* Sectors/FAT */
	__u16	secs_track;	/* Sectors/track */
	__u16	heads;		/* Number of heads */
	__u32	hidden;		/* Number of hidden sectors */
	__u32	total_sect;	/* Number of sectors (if sectors == 0) */

	/* FAT32 only */
	__u32	fat32_length;	/* Sectors/FAT */
	__u16	flags;		/* Bit 8: fat mirroring, low 4: active fat */
	__u8	version[2];	/* Filesystem version */
	__u32	root_cluster;	/* First cluster in root directory */
	__u16	info_sector;	/* Filesystem info sector */
	__u16	backup_boot;	/* Backup boot sector */
	__u16	reserved2[6];	/* Unused */
} __attribute__ ((packed)) boot_sector;

typedef struct volume_info
{
	__u8 drive_number;	/* BIOS drive number */
	__u8 reserved;		/* Unused */
	__u8 ext_boot_sign;	/* 0x29 if fields below exist (DOS 3.3+) */
	__u8 volume_id[4];	/* Volume ID number */
	char volume_label[11];	/* Volume label */
	char fs_type[8];	/* Typically FAT12, FAT16, or FAT32 */
	/* Boot code comes next, all but 2 bytes to fill up sector */
	/* Boot sign comes last, 2 bytes */
} __attribute__ ((packed))volume_info;

typedef struct dir_entry {
	char	name[8],ext[3];	/* Name and extension */
	__u8	attr;		/* Attribute bits */
	__u8	lcase;		/* Case for base and extension */
	__u8	ctime_ms;	/* Creation time, milliseconds */
	__u16	ctime;		/* Creation time */
	__u16	cdate;		/* Creation date */
	__u16	adate;		/* Last access date */	
	__u16	starthi;	/* High 16 bits of cluster in FAT32 */
	__u16	time,date,start;/* Time, date and first cluster */
	__u32	size;		/* File size in bytes */
} __attribute__ ((packed))dir_entry;

typedef struct dir_slot {
	__u8    id;		/* Sequence number for slot */
	__u8    name0_4[10];	/* First 5 characters in name */
	__u8    attr;		/* Attribute byte */
	__u8    reserved;	/* Unused */
	__u8    alias_checksum;/* Checksum for 8.3 alias */
	__u8    name5_10[12];	/* 6 more characters in name */
	__u16   start;		/* Unused */
	__u8    name11_12[4];	/* Last 2 characters in name */
} __attribute__ ((packed))dir_slot;




typedef struct{
	u16 	start;			//开始目录号
	u16 	num;			//目录项个数
	dir_entry *entry;		//缓冲区指针
}dir_t;

typedef struct{
	u16 	start;		//开始
	u16 	num;		//个数
	u16 *clust;			//缓冲区指针
}fat_t;

typedef struct {
	int		fat_size;		/* Size of FAT in bits */
	u16		fat_length;		/* Length of FAT in sectors */
	u8 		fats;	        /*fat number (2)*/
	u16		clust_size;		/* Size of clusters in sectors */
	u16		fat_start;		/* Starting sector of the FAT */
	u16		rootdir_start;	/* Start sector of root directory */
	short	data_start;		/* The sector of the first cluster, can be negative */
	dir_t 	cur_dir;		/* current directory */
	fat_t	cur_fat;		/* current fat */
} fs_t;



/* Currently this doesn't check if the dir exists or is valid... */
s8 init_fat16(void);
int file_fat_cd(const char *path);
int file_fat_ls(void);
s8 file_fat_read(const char *filename, void *buffer,u32 off,u32 size);

#endif /* _FAT_H_ */
