
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




void do_self(u8 key)
{
	PUTCH(key, tty_table[0].read_q);
	wake_up(&tty_table[0].read_q.proc_list);
}

void none(u8 key)
{
}
void ctrl(u8 key)
{
	printk("recive ctrl:%x\n",key);
}
extern void unctrl(u8 key)
{
	printk("recive unctrl:%x\n",key);
}
void alt(u8 key)
{
	printk("recive alt:%x\n",key);
}
void unalt(u8 key)
{
	printk("recive unalt:%x\n",key);
}
void lshift(u8 key)
{
	printk("recive lshift:%x\n",key);
}
void unlshift(u8 key)
{
	printk("recive unlshift:%x\n",key);
}
void rshift(u8 key)
{
	printk("recive rshift:%x\n",key);
}
void unrshift(u8 key)
{
	printk("recive unrshift:%x\n",key);
}

void caps(u8 key)
{
	printk("recive caps:%x\n",key);
}
void uncaps(u8 key)
{
	printk("recive uncaps:%x\n",key);
}


void scroll(u8 key)
{
	printk("recive scroll:%x\n",key);
}
void num(u8 key)
{
	printk("recive num:%x\n",key);
}
void cursor(u8 key)
{
	printk("recive cursor:%x\n",key);
}
void func(u8 key)
{
	printk("recive func:%x\n",key);
}
extern void minus(u8 key)
{
	printk("recive minus:%x\n",key);
}