#include <linux/sched.h>	
#include <linux/kernel.h>	// �ں�ͷ�ļ�������һЩ�ں˳��ú�����ԭ�ζ��塣
#include <signal.h>		// �ź�ͷ�ļ��������źŷ��ų������źŽṹ�Լ��źŲ�������ԭ�͡�
#include <asm/segment.h>	//�β���

extern int sys_exit (int error_code);

// ��ȡ��ǰ�����ź�����λͼ�������룩��
int sys_sgetmask ()
{
	return current->blocked;
}

// �����µ��ź�����λͼ��SIGKILL ���ܱ����Ρ�����ֵ��ԭ�ź�����λͼ��
int sys_ssetmask (int newmask)
{
	int old = current->blocked;
	current->blocked = newmask & ~(1 << (SIGKILL - 1));
	return old;
}

// ����sigaction ���ݵ�fs ���ݶ�to ����fromָ���ں����ݣ�toΪ�û�������
static inline void save_old (char *from,char *to)
{
	int i=sizeof (struct sigaction);
	verify_area (to, i);	// ��֤to �����ڴ��Ƿ��㹻��
	
	while(i--)
		put_fs_byte (*(from++), to++);
}

//���û�����������sigaction���ݵ��ں�
static inline void get_new (char *from, char *to)
{
	int i=sizeof (struct sigaction);
	while(i--)
		*(to++) = get_fs_byte (from++);
}


// ��װ�źŲ�׽����signal()ϵͳ����,������sigaction()
int sys_signal (int signum, long handler, long restorer)
{
	struct sigaction tmp;
	if (signum < 1 || signum > 32 || signum == SIGKILL)	//�ź���Ч
		return -1;
	
	//������action
	tmp.sa_handler = (void (*)(int)) handler;
	//���ֻ��ʹ��һ�Ρ��źŴ����У�����������mask��Ҳ�����������Լ���ָ���ź�
	tmp.sa_flags = SA_ONESHOT | SA_NOMASK;
	tmp.sa_restorer = (void (*)(void)) restorer;//���ص�ַ

	//����ԭaction����������action
	handler = (long) current->sigaction[signum - 1].sa_handler;
	current->sigaction[signum - 1] = tmp;
	return handler;
}

// sigaction()ϵͳ���ã����lodaction!=NULL,�򱣴�ԭ���
//ע��:û�и���flag��NOMASK������sigaction��mask����Ϊ������Ϊ�ⲻ���ϳ���
int sys_sigaction (int signum, const struct sigaction *action,
					struct sigaction *oldaction)
{
	struct sigaction tmp;
	if (signum < 1 || signum > 32 || signum == SIGKILL)	//�ź���Ч
		return -1;

	//����ԭaction
	if (oldaction)
		save_old( (char *)(signum-1+current->sigaction),(char *)oldaction );
	//������action
	get_new ( (char *) action,(char *)(signum-1+current->sigaction) );
	
	return 0;
}
					
//�źŴ��������ڷ����ں˷����û�̬ʱ��������Ч
void do_signal (long eax, long ebx, long ecx, long edx,
			long fs, long es, long ds,long eip, long cs, long eflags, unsigned long *esp, long ss)
{
	unsigned long sa_handler;
	long signr=0,sign;
	long old_eip = eip;
	struct sigaction *sa; 
	int longs;
	unsigned long *tmp_esp;
	//������ǽ�Ҫ�����û�ģʽ��Ҳ���ں�ģʽʱ�������жϵķ��أ��������ź�
	if(!(cs&0x3))
		return ;
	//�޷������źţ��򷵻�(Ӧ����ӷǷ����û�ģʽ���򷵻�)
	sign = current->signal & ~current->blocked;
	if(!sign)
		return ;
	//�����ź�λͼ��ǰ����źţ���ת��Ϊ���к�,��ο�����bsf��btrǶ�����Ż�����������������
	signr = 0;
	while(!(sign&0x1)){
		signr++;
		sign >>= 1;
	}
	current->signal &= ~(1<<signr);//��λ�����ź�

	/*				�����ź�							*/
	sa=&current->sigaction[signr];
	sa_handler=(unsigned long)sa->sa_handler;
	if (sa_handler == 1)//����
		return;
	if (!sa_handler)//Ĭ��
	{
		if (signr == SIGCHLD)
			return;
		else
			sys_exit (1 << (signr));//��ֹ
	}
	// ������źž��ֻ��ʹ��һ�Σ��򽫸þ���ÿ�(���źž���Ѿ�������sa_handler ָ����)��
	if (sa->sa_flags & SA_ONESHOT)
		sa->sa_handler = NULL;
	
	/*			����û���ջ���ں˶�ջ						*/
	eip = sa_handler;//���ص�ַ��Ϊ�źŴ�����
		
	esp -= 9;
	tmp_esp = esp;
	verify_area (esp, 9 * 4);	
// ���û���ջ�д��µ��ϴ��sa_restorer, �ź�signr, ������blocked
// eax, ecx, edx, eflags ���û�����ԭ����ָ�롣
	put_fs_long ((u32) sa->sa_restorer, tmp_esp++);	//sa_resorer
	put_fs_long (signr, tmp_esp++);			//signr
	put_fs_long (current->blocked, tmp_esp++);	//����ԭmask
	put_fs_long (eax, tmp_esp++);				//eax
	put_fs_long (ebx,tmp_esp++);				//Դ����֪��Ϊʲô������ebx
	put_fs_long (ecx, tmp_esp++);				//ecx
	put_fs_long (edx, tmp_esp++);				//edx
	put_fs_long (eflags, tmp_esp++);			//eflags
	put_fs_long (old_eip, tmp_esp++);			//old_eip
	
	//���û������NOMASK,���ʾsa_mask��Ч
	if (!(sa->sa_flags & SA_NOMASK))
		current->blocked |= sa->sa_mask;	// ����������(������)����sa_mask �е���λ��
}


int sys_sigreturn (long ebx, long ecx, long edx,
			long fs, long es, long ds,long eip, long cs, long eflags, unsigned long *esp, long ss)
{
	//�ָ�Ϊ�źŷ���ǰ��״̬
	
	long signr,eax;
	struct sigaction *sa;
	signr=get_fs_long(esp++);
	sa=&current->sigaction[signr];
	
	current->blocked=get_fs_long(esp++);
	eax=get_fs_long(esp++);
	ebx=get_fs_long(esp++);
	ecx=get_fs_long(esp++);
	edx=get_fs_long(esp++);
	eflags=get_fs_long(esp++);
	eip=get_fs_long(esp++);
	return eax;
}

