#include<sys/types.h>
#include<asm/io.h>
#include<linux/kernel.h>
#include<linux/sys.h>
#define	KBD_US
#include<drivers/keyboard.h>
#include<linux/tty.h>


static int shift_flag=0;	//shift��־
static int alt_flag=0;		//alt��־
static int ctrl_flag=0;		//���Ʊ�־
static int cap_flag=0;		//cap��־(��Сд)
static int num_lock=0;		//������

extern struct tty_struct tty_table[3];
void do_self(u8 key)
{
	char c;
	//���ͬʱ����shift��alt��ֻ����alt
	//����shift��caps�������Խ���Сд��ת��
	//����shift��Ч�ڼ�Ϊ���µ��ɿ���capsΪֱ���´ΰ��£��������ɿ�
	
	if(alt_flag){
		c=alt_map[key];
	}else if(shift_flag^cap_flag){
		c=shift_map[key];
	}else{
		c=key_map[key];
	}
	
	if(c=='\r')	//���س�ת��Ϊ����
		c='\n';
	
	//����ǿ�����Ϣ
	if(ctrl_flag){
		switch(c)
		{
		case 'c':sys_kill (-tty_table[0].pgrp,SIGINT);break;//�����ж�<Ctrl>+c
		
		default :printk("not support <Ctrl>+%c\n",c);
		}
		return ;
	}
	
	PUTCH(c, tty_table[0].read_q);
	wake_up(&tty_table[0].read_q.proc_list);
	//��д
	PUTCH(c, tty_table[0].write_q);	//д����
	if(c=='\n')
		PUTCH('\r', tty_table[0].write_q);	//д����
	tty_table[0].write(&tty_table[0]);
		
}

void none(u8 key)
{
}
void ctrl(u8 key)
{
	ctrl_flag=1;
}
extern void unctrl(u8 key)
{
	ctrl_flag=0;
}
void alt(u8 key)
{
	alt_flag=1;
}
void unalt(u8 key)
{
	alt_flag=0;
}
void lshift(u8 key)
{
	shift_flag=1;
}
void unlshift(u8 key)
{
	shift_flag=0;
}
void rshift(u8 key)
{
	shift_flag=1;
}
void unrshift(u8 key)
{
	shift_flag=0;
}

void caps(u8 key)
{
	cap_flag ^= 1;
}
void uncaps(u8 key)
{
}

void scroll(u8 key)
{
	printk("scorll:%x not support now\n",key);
}
void num(u8 key)
{
	num_lock ^= 1;
}
void cursor(u8 key)
{
	printk("cursor:%x not support now\n",key);
}
void func(u8 key)
{
	printk("func:%x not support now\n",key);
}
void minus(u8 key)
{
	printk("minus:%x not support\n",key);
}
void keyboard_init(void)
{
	shift_flag=0;	//shift��־
	alt_flag=0;		//alt��־
	ctrl_flag=0;	//���Ʊ�־
	cap_flag=0;		//cap��־(��Сд)
	num_lock=0;		//������
}