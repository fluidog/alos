
/* 			内存相关      (保证内存容量与页面大小对齐) 				 		*/
#define	MEMORY	(32*1024*1024)	//内存容量(目前物理支持内存最大为32MB)
int HIGH_MEMORY;	//实际内存大小,读取cmos参数后，在cmos_init()中初始化。
//可分配内存的起始地址，0~LOW_MEM用于内核和缓冲区
#define LOW_MEM	(1*1024*1024)
#define PAGING_MEMORY (MEMORY-LOW_MEM)		// 可分配内存容量
#define PAGING_PAGES (PAGING_MEMORY/PAGE_SIZE)	// 可分配内存页数

#define	VM_START	0xc0000000		//内核虚拟内存起始
#define PAGE_SIZE	(4*1024)		//4k页面






