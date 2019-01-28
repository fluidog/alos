#include<drivers/ide.h>
#include<asm/io.h>
#include<sys/types.h>
#include<linux/kernel.h>

#define OVERSTEP -2

/*	addr:»º³åÇø     lba:Âß¼­ÉÈÇøºÅ size:×Ö½ÚÊý*/
s8 ide_read(void *addr,u32 lba,u32 size)
{
	if(!size)return 0;
	if(lba&0xf0000000)return OVERSTEP;
	outb(0,MASTER_IDE+FEATURES);
	outb((size-1)/512+1,MASTER_IDE+SECTOR_COUNT);	//sector counts
	outb((u8)lba,MASTER_IDE+LBALO);				//lba 0~7
	outb((u8)(lba>>8),MASTER_IDE+LBAMID);		//lba 8~15
	outb((u8)(lba>>16),MASTER_IDE+LBAHI);		//lba 16~23
	outb(0xa0|MODE_LBA|(u8)(lba>>24),MASTER_IDE+DRIVEANDHEAD);	//LBA mode,MASTER_IDE,lba 24~27
	outb(IDE_READ,MASTER_IDE+COMMAND);				//read operate
	
	/*	wait for ready	*/
	u8 status;
	do{
		status=inb(MASTER_IDE+STATUS);
	}while(!(status&DRQ_STAT));
	
	/*	read data		*/
	insw(MASTER_IDE+DATA,addr,(size+1)/2);
	return 0;
}

