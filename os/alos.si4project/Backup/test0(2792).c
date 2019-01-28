#include<unistd.h>

void _start()
{
	int x,y;
	x=fork();
	while(1){	
		for(y=0;y<10000000;y++);
		if(!x) 
			write("sub task(0)");
		if(x) 
			write("father task(>0)");
	}
}
