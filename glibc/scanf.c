#include<unistd.h>
//获得最多count个字符，遇到'\n'提前结束，返回获得字符数
int scanf(char *buf,int count)
{
	int tmp=count;
	char c;
	while(count){
		
		if(read(0,&c,1)<0)
			return -1;
		*buf++=c;
		count--;
		if(c=='\n')
			break;
	}
	return tmp-count;
}

int getchar()
{
	char c;
	if(read(0,&c,1)<0)
		return -1;
	return c;
}
