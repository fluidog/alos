#include<utime.h>
#include<sys/stat.h>
#include<sys/times.h>
#include<signal.h>
int sys_setup();
/*				文件系统				*/
int sys_mount(char * dev_name, char * dir_name);
int sys_umount(char * dev_name);
int sys_sync();

int sys_open(const char * filename,int flag,int mode);
int sys_close(unsigned int fd);
int sys_creat(const char * pathname, int mode);
int sys_mkdir(const char * pathname, int mode);
int sys_rmdir(const char * name);
int sys_link(const char * oldname, const char * newname);
int sys_unlink(const char * name);
int sys_mknod(const char * filename, int mode, int dev);
int sys_dup(unsigned int fildes);
int sys_dup2(unsigned int oldfd, unsigned int newfd);

int sys_read(unsigned int fd,char * buf,int count);
int sys_write(unsigned int fd,char * buf,int count);

int sys_utime(char * filename, utimbuf * times);
int sys_access(const char * filename,int mode);
int sys_lseek(unsigned int fd,off_t offset, int origin);
int sys_chmod(const char * filename,int mode);
int sys_chown(const char * filename,int uid,int gid);
int sys_chdir(const char * filename);
int sys_chroot(const char * filename);
int sys_ustat(int dev, struct ustat * ubuf);
int sys_fstat (unsigned int fd, struct stat *statbuf);
int sys_stat (char *filename, struct stat *statbuf);
int sys_umask (int mask);
int sys_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg);
int sys_setsid();


/*				进程					*/
int sys_alarm(long seconds);
int sys_nice (long increment);
int sys_pause();
int sys_waitpid (pid_t pid, unsigned long *stat, int options);

int sys_fork();
int sys_exit (int error_code);
int sys_kill (int pid, int sig);
int sys_execve(const char * filename, char ** argv, char ** envp);
int sys_brk (unsigned long end_data_seg);

int sys_sigaction (int signum, const struct sigaction *action,
							struct sigaction *oldaction);
int sys_signal (int signum, long handler, long restorer);
int sys_sigreturn (long ebx, long ecx, long edx,
			long fs, long es, long ds,long eip, long cs, long eflags, unsigned long *esp, long ss);
int sys_ssetmask (int newmask);
int sys_sgetmask();

int sys_getuid();//用户
int sys_geteuid();//有效用户
int sys_getgid();	//组
int sys_getegid();//有效组

int sys_getpid();//进程号
int sys_getppid();	//父进程
int sys_getpgrp();//进程组

int sys_setuid (int uid);
int sys_setreuid (int ruid, int euid);
int sys_setgid (int gid);
int sys_setregid (int rgid, int egid);
int sys_setpgid (int pid, int pgid);//进程组id



/*		Time			*/

// 返回从1970 年1 月1 日00:00:00 GMT 开始计时的时间值（秒）。如果tloc 不为null，则时间值
// 也存储在那里。
int sys_time (long *tloc);
// 设置系统时间和日期。参数tptr 是从1970 年1 月1 日00:00:00 GMT 开始计时的时间值（秒）。
// 调用进程必须具有超级用户权限。
int sys_stime (long *tptr);
// 获取当前任务时间。tms 结构中包括用户时间、系统时间、子进程用户时间、子进程系统时间。
int sys_times (struct tms *tbuf);


/*		以下函数暂不支持				*/
extern int sys_uname ();
extern int sys_break();
extern int sys_ptrace();

extern int sys_stty();
extern int sys_gtty();

extern int sys_ftime();
extern int sys_rename();
extern int sys_pipe();
extern int sys_prof();

extern int sys_acct();
extern int sys_phys();
extern int sys_lock();
extern int sys_ioctl();
extern int sys_mpx();

extern int sys_ulimit();

typedef int (*fn_ptr)();



