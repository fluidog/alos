
#include <stdarg.h>
#include <asm/io.h>
#include <ctype.h>
#include <types.h>

void vga_init();
int printf(const char * fmt, ...);


#define VIDEO_SEL 		0x3d4	//�����Ĵ���
#define VIDEO_VAL 		0x3d5	//���ݼĴ���
#define VIDEO_CTRL		0x3d8	//ģʽ���ƼĴ���
#define VIDEO_TONE		0x3d9	//CGA��ɫ��Ĵ���
#define VIDEO_STAT		0x3da	//CGA״̬�Ĵ���
#define VIDEO_CLN_CS	0x3db	//��λ��ʼĴ���
#define VIDEO_SET_CS	0x3dc	//���ù�ʼĴ���

#define VGA_BASE 	0xb8000	//�Դ���ʼ�����ַ
#define VGA_END 	0xA0000	//�Դ���������ַ


/* text mode */
#define RGB_BLACK	0x0	//��ɫ
#define RGB_BLUE	0x1	//��ɫ
#define RGB_GREEN	0x2	//��ɫ
#define RGB_CYAN	0x3	//��ɫ
#define RGB_RED		0x4	//��ɫ
#define RGB_MAGENTA	0x5 //���
#define	RGB_BROWN	0x6	//��ɫ
#define RGB_WHITE	0x7	//��ɫ
#define FLASH		0x80	//��˸
#define LIGHT		0x08	//����


#define BLK_GRE			((RGB_BLACK<<4)|RGB_GREEN) 			//�ڵ�����
#define BLK_GRE_L		((RGB_BLACK<<4)|RGB_GREEN|LIGHT) 	//�ڵ�����(����)
#define BLK_GRE_F		((RGB_BLACK<<4)|RGB_GREEN|FLASH) 	//�ڵ�����(��˸)
#define BLK_RED_F		(((RGB_BLACK<<4)|RGB_RED|FLASH) 	//�ڵ׺��֣���˸��
#define WIT_BLU			((RGB_WHITE<<4)|RGB_BLUE)			//�׵�����
#define WIT_BLU_L		((RGB_WHITE<<4)|RGB_BLUE|LIGHT)		//�׵�����(����)
#define BLK_BLU_L		((RGB_BLACK<<4)|RGB_BLUE|LIGHT)		//�ڵ�����(����)

#define MODE_TEXT 0
#define CLOUMN_NUM 80
#define ROW_NUM	25
