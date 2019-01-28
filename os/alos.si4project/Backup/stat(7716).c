#include<linux/kernel.h>
#include<sys/stat.h>
int sys_stat (char *filename, struct stat *statbuf)
{
	panic("not support");
}
int sys_fstat (unsigned int fd, struct stat *statbuf)
{
	panic("not support");

}
