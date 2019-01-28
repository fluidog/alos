/*
 *  linux/kernel/tty_io.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * 'tty_io.c' gives an orthogonal feeling to tty's, be they consoles
 * or rs-channels. It also implements echoing, cooked mode etc.
 *
 * Kill-line thanks to John T Kohl.
 */

#include<linux/tty.h>
#include<asm/segment.h>

#define ALRMMASK (1<<(SIGALRM-1))
#define KILLMASK (1<<(SIGKILL-1))
#define INTMASK (1<<(SIGINT-1))
#define QUITMASK (1<<(SIGQUIT-1))
#define TSTPMASK (1<<(SIGTSTP-1))


#define _L_FLAG(tty,f)	((tty)->termios.c_lflag & f)
#define _I_FLAG(tty,f)	((tty)->termios.c_iflag & f)
#define _O_FLAG(tty,f)	((tty)->termios.c_oflag & f)

#define L_CANON(tty)	_L_FLAG((tty),ICANON)
#define L_ISIG(tty)	_L_FLAG((tty),ISIG)
#define L_ECHO(tty)	_L_FLAG((tty),ECHO)
#define L_ECHOE(tty)	_L_FLAG((tty),ECHOE)
#define L_ECHOK(tty)	_L_FLAG((tty),ECHOK)
#define L_ECHOCTL(tty)	_L_FLAG((tty),ECHOCTL)
#define L_ECHOKE(tty)	_L_FLAG((tty),ECHOKE)

#define I_UCLC(tty)	_I_FLAG((tty),IUCLC)
#define I_NLCR(tty)	_I_FLAG((tty),INLCR)
#define I_CRNL(tty)	_I_FLAG((tty),ICRNL)
#define I_NOCR(tty)	_I_FLAG((tty),IGNCR)

#define O_POST(tty)	_O_FLAG((tty),OPOST)
#define O_NLCR(tty)	_O_FLAG((tty),ONLCR)
#define O_CRNL(tty)	_O_FLAG((tty),OCRNL)
#define O_NLRET(tty)	_O_FLAG((tty),ONLRET)
#define O_LCUC(tty)	_O_FLAG((tty),OLCUC)

struct tty_struct tty_table[3];


void tty_init(void)
{
	//rs_init();
	con_init();

	/*		tty0初始化				*/
	tty_table[0].write=con_write;
	tty_table[0].read_q.head=0;
	tty_table[0].read_q.tail=0;
	tty_table[0].write_q.head=0;
	tty_table[0].write_q.tail=0;
	tty_table[0].secondary.head=0;
	tty_table[0].secondary.tail=0;
	
}

void tty_intr(struct tty_struct * tty, int mask)
{
	
}

static void sleep_if_empty(struct tty_queue * queue)
{
	while(EMPTY(*queue))
		interruptible_sleep_on(&queue->proc_list);
}

static void sleep_if_full(struct tty_queue * queue)
{
	if(!FULL(*queue))
		return;

	while(LEFT(*queue)<128)
		interruptible_sleep_on(&queue->proc_list);

}

void wait_for_keypress(void)
{
	
}

void copy_to_cooked(struct tty_struct * tty)
{
	
}


int tty_read(unsigned channel, char * buf, int nr)
{
	struct tty_struct * tty;
	char c, * b=buf;
	int minimum,time,flag=0;


	if (channel>2 || nr<0) return -1;
	tty = &tty_table[channel];
	
	while (nr>0) {
		sleep_if_empty(&tty->read_q);

		GETCH(tty->read_q,*b++);
		nr--;
	}
	return (b-buf);
}


int tty_write(unsigned channel, char * buf, int nr)
{
	struct tty_struct * tty;
	char c,*b=buf;

	if (channel>2 || nr<0) return -1;
	tty = channel + tty_table;
	while (nr>0) {
		sleep_if_full(&tty->write_q);//目前无意义
	
		while(nr > 0 && !FULL (tty->write_q))
		{
			c=get_fs_byte(b);//从用户空间取得一字节数据
			
			PUTCH(c,tty->write_q);
			if(c=='\n')		//将'\n'转换为'\n'+'\r'
				PUTCH('\r', tty->write_q);
			
			nr--;
			b++;
		}
		tty->write(tty);		
	}
	return (b-buf);
}

/*
 * Jeh, sometimes I really like the 386.
 * This routine is called from an interrupt,
 * and there should be absolutely no problem
 * with sleeping even in an interrupt (I hope).
 * Of course, if somebody proves me wrong, I'll
 * hate intel for all time :-). We'll have to
 * be careful and see to reinstating the interrupt
 * chips before calling this, though.
 *
 * I don't think we sleep here under normal circumstances
 * anyway, which is good, as the task sleeping might be
 * totally innocent.
 */
void do_tty_interrupt(int tty)
{
	copy_to_cooked(tty_table+tty);
}

void chr_dev_init(void)
{
}
