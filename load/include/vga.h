
#include <stdarg.h>
#include <asm/io.h>
#include <ctype.h>
#include <types.h>

void vga_init();
int printf(const char * fmt, ...);


#define VIDEO_SEL 		0x3d4	//索引寄存器
#define VIDEO_VAL 		0x3d5	//数据寄存器
#define VIDEO_CTRL		0x3d8	//模式控制寄存器
#define VIDEO_TONE		0x3d9	//CGA调色板寄存器
#define VIDEO_STAT		0x3da	//CGA状态寄存器
#define VIDEO_CLN_CS	0x3db	//复位光笔寄存器
#define VIDEO_SET_CS	0x3dc	//设置光笔寄存器

#define VGA_BASE 	0xb8000	//显存起始物理地址
#define VGA_END 	0xA0000	//显存结束物理地址


/* text mode */
#define RGB_BLACK	0x0	//黑色
#define RGB_BLUE	0x1	//蓝色
#define RGB_GREEN	0x2	//绿色
#define RGB_CYAN	0x3	//青色
#define RGB_RED		0x4	//红色
#define RGB_MAGENTA	0x5 //洋红
#define	RGB_BROWN	0x6	//棕色
#define RGB_WHITE	0x7	//白色
#define FLASH		0x80	//闪烁
#define LIGHT		0x08	//高亮


#define BLK_GRE			((RGB_BLACK<<4)|RGB_GREEN) 			//黑底绿字
#define BLK_GRE_L		((RGB_BLACK<<4)|RGB_GREEN|LIGHT) 	//黑底绿字(高亮)
#define BLK_GRE_F		((RGB_BLACK<<4)|RGB_GREEN|FLASH) 	//黑底绿字(闪烁)
#define BLK_RED_F		(((RGB_BLACK<<4)|RGB_RED|FLASH) 	//黑底红字（闪烁）
#define WIT_BLU			((RGB_WHITE<<4)|RGB_BLUE)			//白底蓝字
#define WIT_BLU_L		((RGB_WHITE<<4)|RGB_BLUE|LIGHT)		//白底蓝字(高亮)
#define BLK_BLU_L		((RGB_BLACK<<4)|RGB_BLUE|LIGHT)		//黑底蓝字(高亮)

#define MODE_TEXT 0
#define CLOUMN_NUM 80
#define ROW_NUM	25
