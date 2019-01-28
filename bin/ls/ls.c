asm("jmp ls");
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<fs.h>
#include<stdio.h>


char buf[1024];
//��ʾ��ǰĿ¼����
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
		if(size=read(fd,buf,1024)){//��������
			de=(ext2_dir_entry_2 *)buf;
			while(1){
				if((char *)de>=buf+1024 || !de->rec_len)
					break;
				name=de->name;
				name_len=de->name_len;
				//���ָ�����ȵ��ļ���
				while(name_len--)
					putc(*name++);
				putc('\t');//ÿ���ļ���֮����ӷָ���
				de=(ext2_dir_entry_2 *)((unsigned int)de+de->rec_len);	
			}
		}
		else{
			putc('\n');
			exit(0);
		}
	}
}




