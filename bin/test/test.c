asm("jmp _start");
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sdtio.h>
void _start()
{
        int x,y,z;
        char buf[5];
	//while(1){
	for(x=0;x<10000000;x++);
	printf("i am execve made\n");
	//}
	exit(0);
}

