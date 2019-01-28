asm("jmp ls");
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<fs.h>
#include<stdio.h>


char buf[1024];
//显示当前目录内容
void ls(void)
{
	ext2_dir_entry_2 * de;
	int fd,size,name_len;
	char *name;
	if((fd=open(".",O_RDONLY,0777))<0){
		printf("open current dir error\n");
		exit(0);
	}
	while(1){
		if(size=read(fd,buf,1024)){//读到数据
			de=(ext2_dir_entry_2 *)buf;
			while(1){
				if((char *)de>=buf+1024 || !de->rec_len)
					break;
				name=de->name;
				name_len=de->name_len;
				//输出指定长度的文件名
				while(name_len--)
					putc(*name++);
				putc('\t');//每个文件名之间添加分隔符
				de=(ext2_dir_entry_2 *)((unsigned int)de+de->rec_len);	
			}
		}
		else{
			putc('\n');
			exit(0);
		}
	}
}




