#include<types.h>
#include<stddef.h>


#define	VMALLOC_START 	0xc0080000
#define VMALLOC_END		0xc00a0000
#define BLOCK_SIZE 	0x10 //分配最小单元
#define MAX_SIZE 	0x1000//分配最大单元

#define BLOCK_NUM ((VMALLOC_END-VMALLOC_START)/BLOCK_SIZE)
#define BN2VA(blockNum)	(void *)((blockNum)*BLOCK_SIZE+VMALLOC_START)
#define VA2BN(virAddr)	((((u32)virAddr)-VMALLOC_START)/BLOCK_SIZE)

typedef struct{
	u8 mode;
	u8 blockNum; //最大为0，表示0x100
}block_desc_t;
/*  mode:7--是否使用 0~3--未使用字节数 4~6--使用次数,每次分配加一，到8归0*/
#define BLOCK_USED		0x80
#define BLOCK_TIMES		0x70//not support now
#define BLOCK_FREESIZE	0x0f


void init_vmalloc_area(void);
void* malloc(u32 len);
s8 free(void *addr);
void display_vmalloc_area(void);