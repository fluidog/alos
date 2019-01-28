#include<linux/tty.h>
#include<drivers/vga.h>

extern void keyboard_init(void);
void con_write(struct tty_struct * tty)
{
	char c;
	while(!EMPTY(tty->write_q))
	{
		GETCH(tty->write_q, c);
		vga_write(c);
	}
}
void con_init(void)
{
	vga_init();
	keyboard_init();
}


