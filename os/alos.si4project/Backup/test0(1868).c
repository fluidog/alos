#include<unistd.h>

void _start()
{
	int x,y;
	x=fork();
	for(y=1000000;y>0;y--){
		if(!x) 
			//x=1;//
			write("sub task(0)");
		if(x) 
			//x=2;//
			write("father task(>0)");
	}
}
