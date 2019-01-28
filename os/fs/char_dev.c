#include <errno.h>
#include <sys/types.h>

#include <linux/sched.h>
#include <linux/kernel.h>

#include <asm/segment.h>
#include <asm/io.h>


typedef int (*crw_ptr)(int rw,unsigned minor,char * buf,int count,off_t * pos);

//4���豸,ʹ�����豸����Ϊ��Ϊ�ն�
//// �����ն˶�д����������
// ������rw - ��д���minor - �ն����豸�ţ�buf - ��������cout - ��д�ֽ�����
//       pos - ��д������ǰָ�룬�����ն˲�������ָ�����á�
// ���أ�ʵ�ʶ�д���ֽ�����
static int rw_ttyx(int rw,unsigned minor,char * buf,int count,off_t * pos)
{
	return ((rw==READ)?tty_read(minor,buf,count):
		tty_write(minor,buf,count));
}

//5���豸��ʹ�ý���Ĭ���ն�
static int rw_tty(int rw,unsigned minor,char * buf,int count, off_t * pos)
{
	// ������û�ж�Ӧ�Ŀ����նˣ��򷵻س���š�
	if (current->tty<0)
		return -EPERM;
	// ��������ն˶�д����rw_ttyx()��������ʵ�ʶ�д�ֽ�����
	return rw_ttyx(rw,current->tty,buf,count,pos);
}


static int rw_ram(int rw,char * buf, int count, off_t *pos)
{
	return -EIO;
}

static int rw_mem(int rw,char * buf, int count, off_t * pos)
{
	return -EIO;
}

static int rw_kmem(int rw,char * buf, int count, off_t * pos)
{
	return -EIO;
}

static int rw_port(int rw,char * buf, int count, off_t * pos)
{
	int i=*pos;

	while (count-->0 && i<65536) {
		if (rw==READ)
			put_fs_byte(inb(i),buf++);
		else
			outb(get_fs_byte(buf++),i);
		i++;
	}
	i -= *pos;
	*pos += i;
	return i;
}

static int rw_memory(int rw, unsigned minor, char * buf, int count, off_t * pos)
{
	switch(minor) {
		case 0:
			return rw_ram(rw,buf,count,pos);
		case 1:
			return rw_mem(rw,buf,count,pos);
		case 2:
			return rw_kmem(rw,buf,count,pos);
		case 3:
			return (rw==READ)?0:count;	/* rw_null */
		case 4:
			return rw_port(rw,buf,count,pos);
		default:
			return -EIO;
	}
}

#define NRDEVS ((sizeof (crw_table))/(sizeof (crw_ptr)))

// �ַ��豸��д����ָ���
static crw_ptr crw_table[]={
	NULL,		/* ���豸(���豸) */
	rw_memory,	/* /dev/mem �� */
	NULL,		/* /dev/fd ���� */
	NULL,		/* /dev/hd Ӳ�� */
	rw_ttyx,	/* /dev/ttyx �����ն� */
	rw_tty,		/* /dev/tty �ն� */
	NULL,		/* /dev/lp ��ӡ�� */
	NULL};		/* δ�����ܵ� */


int rw_char(int rw,int dev, char * buf, int count, off_t * pos)
{
	crw_ptr call_addr;

	if (MAJOR(dev)>=NRDEVS)
		return -ENODEV;
	if (!(call_addr=crw_table[MAJOR(dev)]))
		return -ENODEV;
	return call_addr(rw,MINOR(dev),buf,count,pos);
}
