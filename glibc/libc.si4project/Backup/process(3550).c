#define __LIBRARY__
#include <unistd.h>

//等待任一子进程退出(阻塞方式)
pid_t wait(int *stat)
{
	return waitpid(-1,stat,0);
}

_syscall1(int,exit,int,status)
_syscall3(int,execve,const char *,file,char **,argv,char **,envp)
_syscall0(int,fork)

_syscall3(int,signal,int,sigunm,long,handler,long,resrorer)
_syscall0(int,sigreturn)
_syscall1(int, alarm, int, sec)

_syscall3(pid_t,waitpid,pid_t,pid,int *,stat,int ,options)





