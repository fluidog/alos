#include<asm/io.h>
#include<asm/ide.h>
#include<asm/ibmpc.h>
#include<fat.h>

static fsdata fsdata_fat16;

/* 根据文件名，从fat16文件系统中加载对应文件	*/
#define ERROR_FILESIZE	-2
#define ERROR_FILENAME	-3
#define ERROR_MISSFLAG	-4
s8 load_file(void *loadAddr,char *fileName)
{
	/*	读取DBR	 */
	ide_read(fsdata_fat16.fatbuf,1,0);
	boot_sector *bSector=(boot_sector *)(fsdata_fat16.fatbuf);
	fsdata_fat16.fatlength=bSector->fat_length;
	fsdata_fat16.fat_begin=bSector->reserved;
	fsdata_fat16.clust_size=bSector->cluster_size;
	fsdata_fat16.fatsize=16;
	fsdata_fat16.rootdir_begin=bSector->fats*bSector->fat_length+ \
						bSector->reserved;
	fsdata_fat16.data_begin=fsdata_fat16.rootdir_begin+32;
	

	/*	读根目录	 jsut scan top32 dir*/
	ide_load(fsdata_fat16.fatbuf,32,fsdata_fat16.rootdir_begin);
	dir_entry *dEntry=(dir_entry *)(fsdata_fat16.fatbuf);
	int i,j;
	for(i=0;i<32;i++)
	{
		if(!dEntry[i].name[0])return ERROR_FILENAME;//未找到
		for(j=0;j<11;j++)
		{
			if(fileName[j]!=dEntry[i].name[j])break;
		}
		if(11==j)break;//完全匹配
	}
	if(i==32)return -1;//未找到
	

	/*	读文件 	*/
	u16 fileNextCluster,fileSize,curFatSector;	//fileSize的单位是簇
	fileSize=(dEntry[i].size-1)/(SECTORSIZE*fsdata_fat16.clust_size)+1;
	fileNextCluster=dEntry[i].start;
	
	while(1)
	{
		/* read a clust data*/
		ide_read(loadAddr,fsdata_fat16.clust_size,
				fsdata_fat16.data_begin+(fileNextCluster-2)*fsdata_fat16.clust_size);

		/*read fat for finding next clust number*/
		curFatSector=fileNextCluster/(SECTORSIZE/2)+fsdata_fat16.fat_begin;
		ide_read(fsdata_fat16.fatbuf,1,curFatSector);
		fileNextCluster=*(u16 *)(fsdata_fat16.fatbuf+fileNextCluster%(SECTORSIZE/2)*2);
		if(fileNextCluster>=0xfff8)//末尾
		{
			if(fileSize!=1)return ERROR_FILESIZE;
			return 0;
		}
		loadAddr=(void *)((u32)loadAddr+SECTORSIZE*fsdata_fat16.clust_size);
		fileSize--;
		if(!fileSize)return ERROR_MISSFLAG; 
	}
}

/*	addr:缓冲区 sectorCount:扇区数 lba:逻辑扇区号*/
s8 ide_read(void *addr,u8 sectorCount,u32 lba)
{
	if(lba&0xf0000000)return -1;
	outb(0,MASTER_IDE+FEATURES);
	outb(sectorCount,MASTER_IDE+SECTOR_COUNT);	//sector counts
	outb((u8)lba,MASTER_IDE+LBALO);			//lba 0~7
	outb((u8)(lba>>8),MASTER_IDE+LBAMID);	//lba 8~15
	outb((u8)(lba>>16),MASTER_IDE+LBAHI);	//lba 16~23
	outb(0xa0|MODE_LBA|(u8)(lba>>24),MASTER_IDE+DRIVEANDHEAD);	//LBA mode,MASTER_IDE,lba 24~27
	outb(READ,MASTER_IDE+COMMAND);	//read operate
	
	/*	wait for ready	*/
	u8 status;
	do{
		status=inb(MASTER_IDE+STATUS);
	}while(!(status&DRQ_STAT));
	
	/*	read data		*/
	insw(MASTER_IDE+DATA,addr,256*sectorCount);
	return 0;
}




	
	