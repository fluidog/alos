asm("jmp _start");
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
extern int printf(const char * fmt, ...);
void _start()
{
        int x,y,z;
        char buf[5];
        //x=fork();
	x=0;
        while(1){
                for(y=0;y<10000000;y++);
                if(!x){
			printf("sub task\n");
                }
                if(x){
                       printf("f:d\n");
        	}
        }
}

