#include<linux/tty.h>
#include<drivers/vga.h>

void con_write(struct tty_struct * tty)
{
	char c;
	while(!EMPTY(tty->write_q))
	{
		GETCH(tty->write_q, c);
		vga_write(c);
		if('\n'==c)vga_write('\r');
	}
}
void con_init(void)
{
	vga_init();
}


