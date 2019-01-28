#include<unistd.h>

void _start()
{
	int x;
	x=fork();
	while(1){
		if(!x)
			write("sub task(0)");
		if(x>0)
			write("father task(>0)");
	}
}
