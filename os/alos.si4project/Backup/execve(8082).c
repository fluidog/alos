/*
 *  linux/lib/execve.c
 *
 *  (C) 1991  Linus Torvalds
 */

#define __LIBRARY__
#include <unistd.h>

_syscall3(int,execve,const char *,file,char **,argv,char **,envp)
_syscall0(int,fork)
_syscall1(int,exit,int,status)

_syscall2(int,mkdir,const char *,path,int,mode)
_syscall1(int,rmdir,const char *, path)
_syscall1(int,chdir,const char *, path)



