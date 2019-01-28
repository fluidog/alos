/*
 *  linux/kernel/panic.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * This function is used through-out the kernel (includeinh mm and fs)
 * to indicate a major problem.
 */
#define PANIC

#include <linux/kernel.h>
#include <linux/sched.h>
extern void sys_sync();
void panic(const char * s)
{
	int error_ip;
	asm("mov 4(%%ebp),%0"
		:"=g"(error_ip):);
	printk("Kernel panic in 0x%x: %s\n",error_ip,s);
	if (current == task[0])
		printk("In task[0] - not syncing\n\r");
	else
		sys_sync();
	for(;;);
}
