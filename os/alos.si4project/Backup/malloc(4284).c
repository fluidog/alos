#include<malloc.h>
#include<linux/kernel.h>

static block_desc_t BLOCK[BLOCK_NUM];
void init_vmalloc_area(void)
{
	u16 i;
	for(i=0;i<BLOCK_NUM;i++)
		BLOCK[i].mode=0;//all blocks is not used  & use times is 0
}

void* malloc(u32 len)
{
	if(len>MAX_SIZE|!len)
	{
		printk("malloc size(0x%x) over limit(0x%x)\n",len,MAX_SIZE);
		return NULL;
	}
	u16 blockNum=(len-1)/BLOCK_SIZE+1;
	DEBUG("req blockNum:%x\n",blockNum);
	u16 index,count;
	for(count=0,index=0;index < BLOCK_NUM;)
	{
		
		if(BLOCK[index].mode&BLOCK_USED)
		{
			count=0;
			index+=BLOCK[index].blockNum?BLOCK[index].blockNum:0x100;
		}
		else
		{
			if(index%blockNum!=0 && 0==count)/*align*/
			{
				index++;
				continue;
			}
			count++;
			index++;
			if(count>=blockNum)//找到符合要求的内存块
			{
				BLOCK[index-count].blockNum=(u8)blockNum;
				BLOCK[index-count].mode |= ((0x10-len%BLOCK_SIZE)&0xf); //free size
				BLOCK[index-count].mode |= BLOCK_USED ;//flag

				return BN2VA(index-count);
			}
		}
	}
	printk("vmArea not enough\n");
	return NULL;				
}

s8 free(void *addr)
{
	if( ((u32)addr%BLOCK_SIZE)| \
		((u32)addr>=VMALLOC_END)| \
		((u32)addr<VMALLOC_START)| \
		!(BLOCK[VA2BN(addr)].mode&BLOCK_USED) )
		return -1;
	BLOCK[VA2BN(addr)].mode &= ~BLOCK_USED;
	return 0;
}

void display_vmalloc_area(void)
{
	u16 index,count,blockNum;//count:all used block numbers
	printk("/*************** all used vmArea *****************/\n");
	for(index=0,count=0;index < BLOCK_NUM;)
	{
		if(BLOCK[index].mode&BLOCK_USED)
		{
			blockNum=BLOCK[index].blockNum?BLOCK[index].blockNum:0x100;
			printk("/*strat:0x%x   end:0x%x-1    freesize:0x%x*/\n",\
				BN2VA(index), \
				BN2VA(index+blockNum), \
				BLOCK[index].mode & BLOCK_FREESIZE );
				
			index +=blockNum;
			count +=blockNum;	
		}
		else index++;
	}
	printk("/************** percent(block): %x / %x **************/\n",count,BLOCK_NUM);	
}

void *realloc(void *addr, unsigned int newSize)
{
	if( ((u32)addr%BLOCK_SIZE)| \
		((u32)addr>=VMALLOC_END)| \
		((u32)addr<VMALLOC_START)| \
		!(BLOCK[VA2BN(addr)].mode&BLOCK_USED) )
		return NULL;
	void *newAddr=NULL;
	if((newAddr=malloc(newSize))==NULL)
		return NULL;
	u32 oldSize=BLOCK[VA2BN(addr)].blockNum? \
				BLOCK[VA2BN(addr)].blockNum*BLOCK_SIZE:	\
				0x100*BLOCK_SIZE;
	int i;
	for(i=0;i<newSize && i<oldSize;i++)
		*((char *)newAddr+i)=*((char *)addr+i);
	free(addr);
	return newAddr;
}