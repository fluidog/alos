#include<linux/kernel.h>
#include<errno.h>
#include<fcntl.h>
#include<linux/sched.h>
#include<linux/sys.h>
// 复制文件句柄(描述符)。arg:新句柄的最小值
static int dupfd(unsigned int fd, unsigned int arg)
{
	if (fd >= NR_OPEN || !current->filp[fd])
		return -EBADF;
	if (arg >= NR_OPEN)	//新句柄无效
		return -EINVAL;
	//寻找空闲新句柄。
	while (arg < NR_OPEN)
		if (current->filp[arg])
			arg++;
		else
			break;
	if (arg >= NR_OPEN)
		return -EMFILE;
	
	//复位close_on_exec,即exec新进程会继承此句柄。
	current->close_on_exec &= ~(1<<arg);
	
	//复制句柄
	(current->filp[arg] = current->filp[fd])->f_count++;
	return arg;		// 返回新的文件句柄。
}
//复制文件句柄为指定句柄，如果新句柄已打开，则先关闭
int sys_dup2(unsigned int oldfd, unsigned int newfd)
{
	sys_close(newfd);		
	return dupfd(oldfd,newfd);	
}

// 复制文件句柄系统调用函数。
int sys_dup(unsigned int fildes)
{
	return dupfd(fildes,0);
}


int sys_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	file * filp;
	if (fd >= NR_OPEN || !(filp = current->filp[fd]))
		return -EBADF;
	
	// 根据不同命令cmd 进行分别处理。
	switch (cmd) {
		// 复制文件句柄。
		case F_DUPFD:	
			return dupfd(fd,arg);
	
		//获取/设置文件描述符标志(lose_on_exec_fd)
		case F_GETFD:	
			return (current->close_on_exec>>fd)&1;
		case F_SETFD:	
			if (arg&1)
				current->close_on_exec |= (1<<fd);
			else
				current->close_on_exec &= ~(1<<fd);
			return 0;

		//获取/设置文件状态标志
		case F_GETFL:	
			return filp->f_flags;
		case F_SETFL://设置文件状态和访问模式(根据arg 设置添加、非阻塞标志)。
			filp->f_flags &= ~(O_APPEND | O_NONBLOCK);
			filp->f_flags |= arg & (O_APPEND | O_NONBLOCK);
			return 0;

		//获取/设置文件锁
		case F_GETLK:	case F_SETLK:	case F_SETLKW:	// 未实现。
			return ENOSYS;
		default:
			return -1;
	}
}



