/*
 *  linux/kernel/printk.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * When in kernel-mode, we cannot use printk, as fs is liable to
 * point to 'interesting' things. Make a printk with fs-saving, and
 * all is well.
 */
#include <stdarg.h>
#include <stddef.h>

#include <linux/kernel.h>

static char buf[1024];

extern int vsprintf(char * buf, const char * fmt, va_list args);

int printk(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);

	asm("push %%fs\n"//��fs�л�Ϊ�ں�������,��Ϊtty_write���ȡfs�ε�����
		"push %%ds\n"
		"pop %%fs\n"
		"push %0\n"//tty_write(0, buf, i);
		"push %1\n"
		"push $0\n"
		"call tty_write\n"
		"add $12,%%esp\n"
		"pop %%fs\n"
		::"m"(i),"i"(&buf));

	return i;
}
