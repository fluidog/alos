#include<unistd.h>
//������count���ַ�������'\n'��ǰ���������ػ���ַ���
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
