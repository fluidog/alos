#include<unistd.h>
#include<fcntl.h>

extern int printf(const char * fmt, ...);
void _start()
{
        int x,y,z;
        char buf[5];
        x=fork();
        while(1){
                for(y=0;y<10000000;y++);
                if(!x){
			printf("sub task\n");
                }
                if(x)
                        printf("father task:%d\n",x);
        }
}

