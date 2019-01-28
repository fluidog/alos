/*
 *  linux/kernel/blk_dev/ll_rw.c
 *
 * (C) 1991 Linus Torvalds
 */

/*
 * This handles all read/write requests to block devices
 */
#include <errno.h>
#include <linux/kernel.h>
#include <asm/system.h>


#include<linux/sched.h>
#include<drivers/ide.h>
#include<fat.h>
/*
 * used to wait on when there are no free requests
 */
task_struct * wait_for_request = NULL;


static inline void lock_buffer(struct buffer_head * bh)
{
	cli();
	while (bh->b_lock)
		sleep_on(&bh->b_wait);
	bh->b_lock=1;
	sti();
}

static inline void unlock_buffer(struct buffer_head * bh)
{
	if (!bh->b_lock)
		printk("ll_rw_block.c: buffer not locked\n\r");
	bh->b_lock = 0;
	wake_up(&bh->b_wait);
}

/*
 * add-request adds a request to the linked list.
 * It disables interrupts so that it can muck with the
 * request-lists in peace.
 */
/*static void add_request(struct blk_dev_struct * dev, struct request * req)
{

}

static void make_request(int major,int rw, struct buffer_head * bh)
{
	
}
*/

void ll_rw_block(int rw, buffer_head * bh)
{
	printk("dev:0x%02x block:%d\n",bh->b_dev,bh->b_blocknr);
	if(bh->b_dev==IDE_DRIVER){
		if(rw==READ){
			lock_buffer(bh);
			if(ide_read(bh->b_data,bh->b_blocknr*2,2)<0)
				panic("ide_read error");
			unlock_buffer(bh);
			bh->b_uptodate=1;

		}else if(rw==WRITE){
			lock_buffer(bh);
			if(ide_write(bh->b_data,bh->b_blocknr*2)<0)
				panic("ide_write");
			if(ide_write(bh->b_data+512,bh->b_blocknr*2+1)<0)
				panic("ide_write");
			unlock_buffer(bh);
			bh->b_dirt=0;

		}else{
			panic("not support other operate");

		}
	}else{
		panic("not support other device");
	}
	
}

/*void blk_dev_init(void)
{
	int i;

	for (i=0 ; i<NR_REQUEST ; i++) {
		request[i].dev = -1;
		request[i].next = NULL;
	}
}*/
