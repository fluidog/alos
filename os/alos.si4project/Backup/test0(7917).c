#include<unistd.h>

void _start()
{
	int x,y,z;
	char buf[5];
	x=fork();
	while(1){	
		for(y=0;y<10000000;y++);
		if(!x){
			read(0,buf,2);
			for(z=100;z>0;z--)
				write("sub task(0)");
		}
		if(x) 
			write("father task(>0)");
	}
}
