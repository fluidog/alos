#include<unistd.h>

_syscall3(int,open,const char *,path,int,flag,int,mode)
_syscall3(int,write,int,fd,char *,buf,off_t,count)
_syscall3(int,read,int,fd,char *,buf,off_t,count)
_syscall2(int,access,const char *,filename, mode_t, mode)
_syscall2(int,mkdir,const char *,path,int,mode)
_syscall1(int,rmdir,const char *, path)
_syscall1(int,chdir,const char *, path)

