#include<utime.h>
int sys_setup();
int sys_exit();
int sys_fork();

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


/*				进程					*/
int sys_getuid();//用户
int sys_geteuid();//有效用户
int sys_getgid();	//组
int sys_getegid();//有效组

int sys_getpid();//进程号
int sys_getppid();	//父进程
int sys_getpgrp();//进程组

int sys_setuid();
int sys_setreuid();
int sys_setgid();
int sys_setregid();
int sys_setpgid();//进程组id

int sys_execve(const char * filename, char ** argv, char ** envp);




extern int sys_waitpid();

extern int sys_time();

extern int sys_break();
extern int sys_stat();
extern int sys_stime();
extern int sys_ptrace();
extern int sys_alarm();
extern int sys_fstat();
extern int sys_pause();

extern int sys_stty();
extern int sys_gtty();


extern int sys_nice();
extern int sys_ftime();
extern int sys_kill();
extern int sys_rename();

extern int sys_dup();
extern int sys_pipe();
extern int sys_times();
extern int sys_prof();
extern int sys_brk();

extern int sys_signal();

extern int sys_acct();
extern int sys_phys();
extern int sys_lock();
extern int sys_ioctl();
extern int sys_fcntl();
extern int sys_mpx();


extern int sys_ulimit();
extern int sys_uname();
extern int sys_umask();

extern int sys_dup2();

extern int sys_setsid();
extern int sys_sigaction();
extern int sys_sgetmask();
extern int sys_ssetmask();


typedef int (*fn_ptr)();

fn_ptr sys_call_table[] = { sys_setup, sys_exit, sys_fork, sys_read,
sys_write, sys_open, sys_close, sys_waitpid, sys_creat, sys_link,
sys_unlink, sys_execve, sys_chdir, sys_time, sys_mknod, sys_chmod,
sys_chown, sys_break, sys_stat, sys_lseek, sys_getpid, sys_mount,
sys_umount, sys_setuid, sys_getuid, sys_stime, sys_ptrace, sys_alarm,
sys_fstat, sys_pause, sys_utime, sys_stty, sys_gtty, sys_access,
sys_nice, sys_ftime, sys_sync, sys_kill, sys_rename, sys_mkdir,
sys_rmdir, sys_dup, sys_pipe, sys_times, sys_prof, sys_brk, sys_setgid,
sys_getgid, sys_signal, sys_geteuid, sys_getegid, sys_acct, sys_phys,
sys_lock, sys_ioctl, sys_fcntl, sys_mpx, sys_setpgid, sys_ulimit,
sys_uname, sys_umask, sys_chroot, sys_ustat, sys_dup2, sys_getppid,
sys_getpgrp, sys_setsid, sys_sigaction, sys_sgetmask, sys_ssetmask,
sys_setreuid,sys_setregid };



