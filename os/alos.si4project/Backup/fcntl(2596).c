#include<linux/kernel.h>
#include<errno.h>
#include<fcntl.h>
#include<linux/sched.h>
#include<linux/sys.h>
// �����ļ����(������)��arg:�¾������Сֵ
static int dupfd(unsigned int fd, unsigned int arg)
{
	if (fd >= NR_OPEN || !current->filp[fd])
		return -EBADF;
	if (arg >= NR_OPEN)	//�¾����Ч
		return -EINVAL;
	//Ѱ�ҿ����¾����
	while (arg < NR_OPEN)
		if (current->filp[arg])
			arg++;
		else
			break;
	if (arg >= NR_OPEN)
		return -EMFILE;
	
	//��λclose_on_exec,��exec�½��̻�̳д˾����
	current->close_on_exec &= ~(1<<arg);
	
	//���ƾ��
	(current->filp[arg] = current->filp[fd])->f_count++;
	return arg;		// �����µ��ļ������
}
//�����ļ����Ϊָ�����������¾���Ѵ򿪣����ȹر�
int sys_dup2(unsigned int oldfd, unsigned int newfd)
{
	sys_close(newfd);		
	return dupfd(oldfd,newfd);	
}

// �����ļ����ϵͳ���ú�����
int sys_dup(unsigned int fildes)
{
	return dupfd(fildes,0);
}


int sys_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	file * filp;
	if (fd >= NR_OPEN || !(filp = current->filp[fd]))
		return -EBADF;
	
	// ���ݲ�ͬ����cmd ���зֱ���
	switch (cmd) {
		// �����ļ������
		case F_DUPFD:	
			return dupfd(fd,arg);
	
		//��ȡ/�����ļ���������־(lose_on_exec_fd)
		case F_GETFD:	
			return (current->close_on_exec>>fd)&1;
		case F_SETFD:	
			if (arg&1)
				current->close_on_exec |= (1<<fd);
			else
				current->close_on_exec &= ~(1<<fd);
			return 0;

		//��ȡ/�����ļ�״̬��־
		case F_GETFL:	
			return filp->f_flags;
		case F_SETFL://�����ļ�״̬�ͷ���ģʽ(����arg ������ӡ���������־)��
			filp->f_flags &= ~(O_APPEND | O_NONBLOCK);
			filp->f_flags |= arg & (O_APPEND | O_NONBLOCK);
			return 0;

		//��ȡ/�����ļ���
		case F_GETLK:	case F_SETLK:	case F_SETLKW:	// δʵ�֡�
			return ENOSYS;
		default:
			return -1;
	}
}



