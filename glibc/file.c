#include<unistd.h>

_syscall3(int,open,const char *,path,int,flag,int,mode)
_syscall3(int,write,int,fd,char *,buf,off_t,count)
_syscall3(int,read,int,fd,char *,buf,off_t,count)
_syscall2(int,access,const char *,filename, mode_t, mode)
_syscall2(int,mkdir,const char *,path,mode_t,mode)
_syscall1(int,rmdir,const char *, path)
_syscall2(int,link,const char *,oldname,const char *,newname)
_syscall1(int,unlink,const char *, name)
_syscall3(int,mknod,const char *,filename,mode_t,mode,dev_t,dev)

_syscall1(int,dup,int, fildes)
_syscall2(int,dup2,int,oldfd,int,newfd)

_syscall2(int,utime,const char *,filename,utimbuf *,times)
_syscall2(int,ustat,dev_t,dev,struct ustat *,ubuf)

_syscall1(int,chdir,const char *, path)
_syscall2(int,chmod,const char *,filename,mode_t,mode)
_syscall3(int,chown,const char *,filename,uid_t,uid,gid_t,gid)
_syscall1(int,chroot,const char *,filename)

_syscall0(int,setsid)


