/*
 *  linux/lib/write.c
 *
 *  (C) 1991  Linus Torvalds
 */

#define __LIBRARY__
#include <unistd.h>

//_syscall3(int,write,int,fd,const char *,buf,off_t,count)

_syscall1(int,write,const char*,buf);
_syscall3(int,read,unsigned,c,char *,buf,int,n);

