asm("jmp shell");
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<signal.h>
#include<stdio.h>


#define NR_BUILTIN (sizeof(builtin)/sizeof(const char *))
/*
		shell 内建命令
0:	cd
1:	mkdir
2:	rmdir
*/
static int built_in(char *execArgv[])
{
	int i;
	const char *builtin[]={"cd","mkdir","rmdir"};//内建命令
	for(i=0;i<NR_BUILTIN;i++)
		if(!strcmp(execArgv[0],builtin[i]))	//目前未除路径
			break;
		
	if(i==NR_BUILTIN)	//不是内建命令
		return -1;
	//如果匹配内建shell命令
	switch(i){
		case 0:chdir(execArgv[1]);		break;
		case 1:mkdir(execArgv[1],0777);	break;
		case 2:rmdir(execArgv[1]);		break;
		
		default :return -1;		//未处理
	}
	return 0;
}

int do_command(char *execArgv[])
{
	int ret,ebp;
	char path[30]="/bin/";	//默认执行文件路径/bin/
	if(!built_in(execArgv))		//内建命令
		return 0;
	strcat(path,execArgv[0]);	//添加执行路径	
	
	ret=fork();
	if(ret<0){
		printf("fork  error\n");
		while(1);
	}
	if(!ret){//子进程执行命令	
		ret=execve(path,execArgv,NULL);
		printf("execve:%d\n",ret);
		exit(0);
	}
	return 0; //父进程退出
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
	char execBuff[21];//命令缓冲区最多20个字符，加一个结束字符
	char *execArgv[10],*cur;	//最多传递9个参数(包含cmd)
	printf("hello,You are in alos-shell now(press 'h' for help)\n");
	//signal(SIGALRM,(long)sig_test,(long)sig_ret);
	//alarm(1);
	if(!fork()){
		while(1)
		{
			memset(execBuff,0,sizeof(execBuff));
			memset(execArgv,0,sizeof(execArgv));
			
			/*			输入命令和参数					*/	
			if((count=scanf(execBuff,sizeof(execBuff)))<=0)
				printf("scanf error\n");
				//memcpy(execBuff,"ls\n",3);count=3;
			if(count==sizeof(execBuff) && execBuff[count]!='\n'){
				printf("cmd is too long\n");
				continue ;
			}
			if(count==1)	//只输入了回车
				continue ;	
			execBuff[count-1]='\0';//覆盖'\n'为字符串结束标志'\0'
			
			/*		将命令字符串分解为命令+参数格式						*/	
			for(cur=execBuff,i=0;i<9;)
			{					
				if(*cur==' '){//排除多余空格
					cur++;
					continue;
				}
				if(!*cur)	//排除' '后无参数
					break;
		
				execArgv[i++]=cur;
			
				if(cur=strchr(cur,' '))//发现' '分隔符
					*cur++='\0';	//分割字符串
				else
					break;
			}
			execArgv[i]=NULL;//结束符
			
			/*				解析命令						*/
			if(do_command(execArgv)<0)
				printf("not find this command");
		}
	}
	else{//0进程,收尸
		while(1){
			//wait(&wait_stat);
			//sync();
		}
	}
}





