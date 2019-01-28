/*
 *  linux/lib/write.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include<unistd.h>

_syscall3(int,write,int,fd,char *,buf,off_t,count)
_syscall3(int,read,int,fd,char *,buf,off_t,count)
_syscall3(int,open,const char *,path,int,flag,int,mode)
_syscall2(int, access,const char *,filename, mode_t, mode)


