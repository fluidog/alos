#define __LIBRARY__
#include <unistd.h>

sighandler_t signal(int signum, sighandler_t handler)
{
	long _res;

	asm("mov 1f,%%edx\n"
		"int $0x80\n"
		"1:\n"
		"mov %4,%%eax\n"
		"int $0x80\n"
		:"=a"(_res)
		:"0"(__NR_signal),"b"(signum),"c"(handler),"i"(__NR_sigreturn));

	return (sighandler_t)_res;
}

//等待任一子进程退出(阻塞方式)
pid_t wait(int *stat)
{
	return waitpid(-1,stat,0);
}


_syscall1(int,exit,int,status)
_syscall3(int,execve,const char *,file,char **,argv,char **,envp)
_syscall0(int,fork)

_syscall0(int,sigreturn)
_syscall1(int, alarm, int, sec)
_syscall3(pid_t,waitpid,pid_t,pid,int *,stat,int ,options)
_syscall0(int,pause)

_syscall1(int, time, long *, tloc)




