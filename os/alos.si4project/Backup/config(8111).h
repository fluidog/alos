
/* 			�ڴ����      (��֤�ڴ�������ҳ���С����) 				 		*/
#define	MEMORY	(16*1024*1024)	//�ڴ�����(Ŀǰ�����ڴ����Ϊ16MB)
//�ɷ����ڴ����ʼ��ַ��0~LOW_MEM�����ں˺ͻ�����
#define LOW_MEM	(1*1024*1024)
#define PAGING_MEMORY (MEMORY-LOW_MEM)		// �ɷ����ڴ�����
#define PAGING_PAGES (PAGING_MEMORY/PAGE_SIZE)	// �ɷ����ڴ�ҳ��

#define	VM_START	0xc0000000		//�ں������ڴ���ʼ
#define PAGE_SIZE	(4*1024)		//4kҳ��






