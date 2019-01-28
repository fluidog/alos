#define __LIBRARY__
#include <unistd.h>

_syscall0(int,fork)
_syscall1(int,exit,int,status)
_syscall3(int,execve,const char *,file,char **,argv,char **,envp)


_syscall3(int,signal,int,sigunm,long,handler,long,resrorer)
_syscall0(int,sigreturn)
_syscall1(int, alarm, int, sec)
_syscall1(pid_t, wait, int *, wait_stat)




