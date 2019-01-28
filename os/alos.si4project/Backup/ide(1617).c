#include<drivers/ide.h>
#include<asm/io.h>
#include<sys/types.h>
#include<linux/kernel.h>


//blockΪ�߼�����(lba)��,countΪ��������(0��ʾ256)
//		ע��:��������СΪ512B���ϲ㺯��������С����Ϊ1K,�������Լ�ת��
s8 ide_read(char *addr,u32 block,u8 count)
{
	if(block&0xf0000000)
		return -1;
	/*			arg		*/
	outb(0,MASTER_IDE+FEATURES);
	outb(count,MASTER_IDE+SECTOR_COUNT);	//sector counts
	outb((u8)block,MASTER_IDE+LBALO);				//lba 0~7
	outb((u8)(block>>8),MASTER_IDE+LBAMID);		//lba 8~15
	outb((u8)(block>>16),MASTER_IDE+LBAHI);		//lba 16~23
	outb(0xa0|MODE_LBA|(u8)(block>>24),MASTER_IDE+DRIVEANDHEAD);	//LBA mode,MASTER_IDE,lba 24~27
	outb(IDE_READ,MASTER_IDE+COMMAND);				//read operate

	/*	wait for ready	*/
	u8 status;
	do{
		status=inb(MASTER_IDE+STATUS);
	}while(!(status&DRQ_STAT));
	
	/*	read data		*/
	insw(MASTER_IDE+DATA,addr,count*256);//��ȡ����(word)Ϊcount*256
	return 0;
}

//addr:��������ʼ    	block:�����߼�������  			count:������
s8 ide_write(char *addr,u32 block,u8 count)
{
	u8 status;
	if(block&0xf0000000)
		return -1;
	/*			arg		*/
	outb(0,MASTER_IDE+FEATURES);
	outb(count,MASTER_IDE+SECTOR_COUNT);	//sector counts
	outb((u8)block,MASTER_IDE+LBALO);				//lba 0~7
	outb((u8)(block>>8),MASTER_IDE+LBAMID);		//lba 8~15
	outb((u8)(block>>16),MASTER_IDE+LBAHI);		//lba 16~23
	outb(0xa0|MODE_LBA|(u8)(block>>24),MASTER_IDE+DRIVEANDHEAD);	//LBA mode,MASTER_IDE,lba 24~27
	outb(IDE_WRITE,MASTER_IDE+COMMAND);				//write operate

	/*	wait for ready	*/
	
	do{
		status=inb(MASTER_IDE+STATUS);
	}while(!(status&DRQ_STAT));
	
	/*	read data		*/
	outsw(MASTER_IDE+DATA,addr,count*256);
	return 0;
}

