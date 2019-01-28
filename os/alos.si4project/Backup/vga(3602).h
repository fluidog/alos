#include<types.h>
void vga_write(char charactor);
void vga_init();

#define VIDEO_SEL 		0x3d4	//索引寄存器
#define VIDEO_VAL 		0x3d5	//数据寄存器
#define VIDEO_CTRL		0x3d8	//模式控制寄存器
#define VIDEO_TONE		0x3d9	//CGA调色板寄存器
#define VIDEO_STAT		0x3da	//CGA状态寄存器
#define VIDEO_CLN_CS	0x3db	//复位光笔寄存器
#define VIDEO_SET_CS	0x3dc	//设置光笔寄存器

#define VGA_PHY_BASE 	0xb8000	//显存物理地址

typedef struct{
	u8 mode;
	u32 base;
	u32 pos;
	u8 rowNum;
	u8 cloumnNum;
	u8 font;
}vga_t;


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
#define BLK_RED_F		(((RGB_BLACK<<4)|RGB_RED|FLASH) 	//黑底红字（闪烁）
#define WIT_BLU			((RGB_WHITE<<4)|RGB_BLUE)			//白底蓝字
#define WIT_BLU_L		((RGB_WHITE<<4)|RGB_BLUE|LIGHT)		//白底蓝字(高亮)

#define MODE_TEXT 0
#define CLOUMN_NUM 80
#define ROW_NUM	25


















/*char x[]={
0b00000000,
0b00011000,
0b00011000,
0b00011000,
0b00011000,
0b00100100,
0b00100100,
0b00100100,
0b01111110,
0b01000010,
0b01000010,
0b01000010,
0b11100111,
0b00000000,
0b00000000
};*/