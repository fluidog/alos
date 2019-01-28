#include<drivers/ide.h>
#include<fat.h>
#include<string.h>
#include<linux/kernel.h>

/* 根据文件名，从fat16文件系统中加载对应文件	*/


#define ERROR_FILESIZE	-2
#define ERROR_FILENAME	-3
#define ERROR_MISSFLAG	-4

static s8 get_dir(u32 lba);
static s8 get_fat(u16 clust);//默认读一个扇区fat
static int file_fat_short_cd(const char *path);


#define PRINT_NAME(name) do{\
	int tmp_i;\
	for(tmp_i=0;tmp_i<11;tmp_i++){\
		if((name)[tmp_i]<=32){\
			printk("(%2x)",name[tmp_i]);\
		}else{\
			printk("%c",name[tmp_i]);\
		}\
	}\
}while(0)
static fs_t fat16;
static void trans(char *name,char *buf)
{
	int i;
	for(i=0;i<11;i++){
		if(*name<32){
			*buf++='*';
			name++;
		}else if(*name==32){
			name++;
		}else {	
			if(8==i)
				*buf++='.';
			*buf++=*name++;
		}
	}
	*buf='\0';
}
int file_fat_cd(const char * path)
{
	if(*path=='/')		//绝对地址
		if(file_fat_short_cd("/")<0)
			return -1;
	
	char *path_back,*buf; //path的副本
	if((path_back=buf=(char *)malloc(128))==NULL)
		return -1;
	strcpy(path_back, path);
	path_back=strtok(path_back,"/");
	while(path_back){
		if(file_fat_short_cd(path_back)<0){
			free(buf);
			return -1;
		}
		path_back=strtok(NULL,"/");
	}
	free(buf);
	return 0;
}
static int file_fat_short_cd(const char *path)
{
	if(*path=='/'){ 			//进入根目录
		if(get_dir(fat16.rootdir_start)<0)
			return -1;
		return 0;
	}
	dir_entry *entry;
	if((entry=fat16.cur_dir.entry)==NULL)
		return -1;
	int i;
	char name[13];
	for(i=0;i<fat16.cur_dir.num;i++){	
		trans(entry[i].name,name);
		if(!strcmp(name,path)){
			u16 sector;
			sector=(entry[i].start-2)*fat16.clust_size+fat16.data_start;
			if(get_dir(sector)<0)
				return -1;
			return 0;
		}
	}
	return ERROR_FILENAME;
}
int file_fat_ls(void)
{
	dir_entry *entry;
	if((entry=fat16.cur_dir.entry)==NULL)
		return -1;
	int i;
	char name[13];
	for(i=0;i<fat16.cur_dir.num;i++){	
		trans(entry[i].name,name);	
		printk("%s",name);

		//PRINT_NAME(entry[i].name);  //原始名
		printk("\t0x%x\n",entry[i].size);
	}
}
s8 init_fat16(void)
{
	boot_sector *boot=NULL;
	if(!(boot=(boot_sector *)malloc(512))) 
		return -1;
	if(ide_read(boot,0,512)<0) 	//读取boot扇区
		return -1;
	
	//fat16.fat_size=16; //目前没啥用
	fat16.clust_size=boot->cluster_size;	//每簇扇区数
	fat16.fat_length=boot->fat_length;		//fat扇区数
	fat16.fats=boot->fats;					//fat数，fat副本
	fat16.fat_start=boot->reserved;			//保留扇区，fat之前的扇区数，包括引导扇区
	fat16.rootdir_start=fat16.fat_start+fat16.fats*fat16.fat_length;	//根目录开始扇区
	fat16.data_start=fat16.rootdir_start+32;	//数据区开始扇区
	free(boot);
	
	fat16.cur_dir.start=0;
	fat16.cur_dir.num=0;
	fat16.cur_dir.entry=NULL;
	if(get_dir(fat16.rootdir_start)<0)	//初始化根目录到缓冲区
		return -1;

	fat16.cur_fat.start=0;
	fat16.cur_fat.num=0;
	fat16.cur_fat.clust=NULL;
	if(get_fat(0)<0)				//初始化一个扇区的fat到缓冲区
		return -1;
	return 0;	
}
static s8 get_dir(u32 lba) 	//如果目录项扇区不连续，则会出错,小心子目录
{
	fat16.cur_dir.start=0;
	fat16.cur_dir.num=0;
	if(fat16.cur_dir.entry){
		free(fat16.cur_dir.entry);
		fat16.cur_dir.entry=NULL;
	}
	dir_entry *entry=NULL;
	int i=0,dirSize=512;
	if(!(entry=(dir_entry *)malloc(dirSize)))
			return -1;	
	while(1){
		if(ide_read((void*)((u32)entry+dirSize-512),lba++,512)<0)
			return -1;
		for(;i<dirSize/32;i++){		//目录项数
			if(*(char *)(entry+i)){
				fat16.cur_dir.num++;
			}else{
				fat16.cur_dir.entry=entry;
				return 0;
			}
		}
		dirSize+=512;
		if(dirSize>512*32)		//目录项个数最多为32个
			break;
		if((entry=(dir_entry*)realloc(entry,dirSize))==NULL)
			return -1;
	}	
}
static s8 get_fat(u16 clust)//默认读一个扇区fat
{
	if( (clust<fat16.cur_fat.start) || \
			(clust>=fat16.cur_fat.start+fat16.cur_fat.num) ){	

		if(fat16.cur_fat.clust==NULL) 
			if((fat16.cur_fat.clust=(u16 *)malloc(512))==NULL)
				return -1;
			
		fat16.cur_fat.start=clust/256*256;
		fat16.cur_fat.num=512/2; 
				
		if(ide_read(fat16.cur_fat.clust,fat16.fat_start+clust/256,512)<0)
			return -1;
	}
	return 0;
}

#define CLU2SEC(clust) \
			(((clust)-2)*fat16.clust_size+fat16.data_start)//cluest -> sector

/*	buffer:缓冲区 start:文件偏移起始地址   	     size:读取的字节长度	*/
s8 file_fat_read(const char *fileName, void *buffer, u32 start,u32 size)
{
	if(!*fileName)
		return 0;
	if(*fileName=='/')
		file_fat_cd("/");
	
	char *path,*name,*buf;
	if((path=buf=(char *)malloc(128))==NULL)
		return -1;
	strcpy(path, fileName);
	name=strrchr(path,'/');
	if(name){
		*name++='\0';
	}else{
		name=path;
		path=NULL;
	}
	if(path)
		if(file_fat_cd(path)<0)
			return -1;
	dir_entry *entry;
	if((entry=fat16.cur_dir.entry)==NULL)
		return -1;
	int i;
	char name2[13];
	u32 fileSize,clust;
	for(i=0;i<fat16.cur_dir.num;i++){	
		trans(entry[i].name,name2);

		if(!strcmp(name,name2))		//匹配
			break;
	}
	if(i==fat16.cur_dir.num)
		return ERROR_FILENAME;
	fileSize=entry[i].size;
	clust=entry[i].start;

	if(start>fileSize)return 0;		//偏移不能大于文件长度
	if(start+size>fileSize)			//读取长度不能超过文件长度
		size=fileSize-start;
	u32 end;//=start+size;

	if((buf=(char*)malloc(fat16.clust_size*SECTORSIZE))==NULL)
			return -1;
	u32 read=0;
	while(size){
			
		if(ide_read(buf,CLU2SEC(clust),fat16.clust_size*SECTORSIZE)<0)	//read data
			return -1;
			
		read+=fat16.clust_size*SECTORSIZE;

		i=0;
		end=fat16.clust_size*SECTORSIZE;
		
		if(read<=start){
			end=0;
		}	
		if(read>start){
			i=start%(fat16.clust_size*SECTORSIZE);	
		}
		if(read>start+size){
			end=(start+size)%(fat16.clust_size*SECTORSIZE);
		}
		
		size-=(end-i);
		start+=(end-i);

		for(;i<end;i++)
			*(char *)buffer++=buf[i];
		
			
		if(get_fat(clust)<0)				//read fat
			return -1;
	
		/*   	求下一簇    	    */
		clust-=fat16.cur_fat.start;		
		clust=fat16.cur_fat.clust[clust];

		
		if(clust>=0xfff8){
			if(size)		//文件长度错误，提前遇到终结符
				return ERROR_FILESIZE;
			return 0;
		}					
	}
	return 0;
}
	
