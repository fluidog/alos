#include<sys/types.h>
#include<asm/io.h>
#include<linux/kernel.h>

#define	KBD_US
#include<drivers/keyboard.h>
#include<linux/tty.h>

static u8 mode=0;
// mode 是键盘特殊键的按下状态标志。
// 表示大小写转换键(caps)、交换键(alt)、控制键(ctrl)和换档键(shift)的状态。
// 位7 caps 键按下:
// 位6 caps 键的状态(应该与leds 中的对应标志位一样)；
// 位5 右alt 键按下；
// 位4 左alt 键按下；
// 位3 右ctrl 键按下；
// 位2 左ctrl 键按下；
// 位1 右shift 键按下；
// 位0 左shift 键按下。


static int shift_flag=0;	//shift标志
static int alt_flag=0;		//alt标志
static int ctrl_flag=0;		//控制标志
static int cap_flag=0;		//cap标志(大小写)
static int num_lock=0;		//数字锁
void do_self(u8 key)
{
	//应该先处理控制信息
	if(ctrl_flag){
		printk("ctrl not support by now\n");
	}
	char c;
	//如果同时按下shift和alt，只处理alt
	//按下shift和caps，都可以将大小写翻转，
	//但是shift有效期间为按下到松开，caps为直到下次按下，不处理松开
	
	if(alt_flag){
		c=alt_map[key];
	}else if(shift_flag^cap_flag){
		c=shift_map[key];
	}else{
		c=key_map[key];
	}
	
	if(c=='\r')	//将回车转换为换行
		c='\n';
	PUTCH(c, tty_table[0].read_q);
	printk("%c",c);//僵硬的回显，之后会改的，应该加入write_q
	wake_up(&tty_table[0].read_q.proc_list);
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
	shift_flag=0;	//shift标志
	alt_flag=0;		//alt标志
	ctrl_flag=0;	//控制标志
	cap_flag=0;		//cap标志(大小写)
	num_lock=0;		//数字锁
}