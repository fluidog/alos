#include<linux/kernel.h>
int sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	panic("not support");

}

