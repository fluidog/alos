#include<linux/kernel.h>
int sys_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	panic("not support");
}
int sys_dup2(unsigned int oldfd, unsigned int newfd)
{
	panic("not support");
}
int sys_dup(unsigned int fildes)
{
	panic("not support");

}
