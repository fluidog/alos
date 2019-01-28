asm("jmp shell");
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<signal.h>
#include<stdio.h>


#define NR_BUILTIN (sizeof(builtin)/sizeof(const char *))
/*
		shell �ڽ�����
0:	cd
1:	mkdir
2:	rmdir
*/
static int built_in(char *execArgv[])
{
	int i;
	const char *builtin[]={"cd","mkdir","rmdir"};//�ڽ�����
	for(i=0;i<NR_BUILTIN;i++)
		if(!strcmp(execArgv[0],builtin[i]))	//Ŀǰδ��·��
			break;
		
	if(i==NR_BUILTIN)	//�����ڽ�����
		return -1;
	//���ƥ���ڽ�shell����
	switch(i){
		case 0:chdir(execArgv[1]);		break;
		case 1:mkdir(execArgv[1],0777);	break;
		case 2:rmdir(execArgv[1]);		break;
		
		default :return -1;		//δ����
	}
	return 0;
}

int do_command(char *execArgv[])
{
	int ret,ebp;
	char path[30]="/bin/";	//Ĭ��ִ���ļ�·��/bin/
	if(!built_in(execArgv))		//�ڽ�����
		return 0;
	strcat(path,execArgv[0]);	//���ִ��·��	
	
	ret=fork();
	if(ret<0){
		printf("fork  error\n");
		while(1);
	}
	if(!ret){//�ӽ���ִ������	
		ret=execve(path,execArgv,NULL);
		printf("execve:%d\n",ret);
		exit(0);
	}
	return 0; //�������˳�
}
void sig_test(int signal)
{
	printf("signal:%d\n",signal);	
}
void sig_ret(void);
asm(".text\n .global sig_ret\n"
	"sig_ret:\n"
	"mov $72,%eax\n"
	"int $0x80");


void shell(int argc,char **argv,char **env)
{
	int count,wait_stat;
	int i=0,j;
	char execBuff[21];//����������20���ַ�����һ�������ַ�
	char *execArgv[10],*cur;	//��ഫ��9������(����cmd)
	printf("hello,You are in alos-shell now(press 'h' for help)\n");
	//signal(SIGALRM,(long)sig_test,(long)sig_ret);
	//alarm(1);
	if(!fork()){
		while(1)
		{
			memset(execBuff,0,sizeof(execBuff));
			memset(execArgv,0,sizeof(execArgv));
			
			/*			��������Ͳ���					*/	
			if((count=scanf(execBuff,sizeof(execBuff)))<=0)
				printf("scanf error\n");
				//memcpy(execBuff,"ls\n",3);count=3;
			if(count==sizeof(execBuff) && execBuff[count]!='\n'){
				printf("cmd is too long\n");
				continue ;
			}
			if(count==1)	//ֻ�����˻س�
				continue ;	
			execBuff[count-1]='\0';//����'\n'Ϊ�ַ���������־'\0'
			
			/*		�������ַ����ֽ�Ϊ����+������ʽ						*/	
			for(cur=execBuff,i=0;i<9;)
			{					
				if(*cur==' '){//�ų�����ո�
					cur++;
					continue;
				}
				if(!*cur)	//�ų�' '���޲���
					break;
		
				execArgv[i++]=cur;
			
				if(cur=strchr(cur,' '))//����' '�ָ���
					*cur++='\0';	//�ָ��ַ���
				else
					break;
			}
			execArgv[i]=NULL;//������
			
			/*				��������						*/
			if(do_command(execArgv)<0)
				printf("not find this command");
		}
	}
	else{//0����,��ʬ
		while(1){
			//wait(&wait_stat);
			//sync();
		}
	}
}





