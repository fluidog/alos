#include<drivers/vga.h>	//VGA相关参数
#include<asm/io.h>	//端口输入输出	
#include<ctype.h>	//判断字符类型

#include<linux/config.h>	
#include<asm/system.h>
#include<string.h>

static struct{
	u8 mode;	//模式(no used)
	u32 pos;	//光标
	u32 origin;	//显存起始(指正在显示的内存)
	u32 end;	//显存尾端
	u8 rowNum;	//行
	u8 cloumnNum;//列
	u32 size;	//一屏内存
	u8 font;	//字体
}vga;

static inline  void set_curson(void);
static inline  void get_curson(void);
static void scrup (void);
static inline  void set_orgin(void);


static void puts(char *s)
{
	while(*s){
		vga_write(*s);
		s++;
	}
}

void vga_write(char charactor)
{
	int i;
	if(isprint(charactor))
	{
		if(vga.pos>=vga.end)//如果将超出屏幕
			scrup();
		//输出一个可显示字符
		*(char *)(vga.pos)=charactor;
		*(char *)(vga.pos+1)=vga.font;
		vga.pos+=2;
		set_curson();
		return ;
	}
	
	if(iscntrl(charactor))
	{
		switch (charactor)
		{
			case '\n':i=vga.cloumnNum;
					while(i--)vga_write(' ');break;
			case '\t':i=4;	//默认4个空格
					while(i--)vga_write(' ');break;
			case '\r':vga.pos -= (vga.pos-VGA_BASE)%(vga.cloumnNum*2);break;
			default :puts("\nctrl char not support now!!\n");
		}
		set_curson();
		return ;
	}	
	puts("\nerror charactor!!\n");
}


static inline void set_curson(void)
{
	outb_p(14, VIDEO_SEL);
	outb_p((unsigned char)(0xff & ((vga.pos- VGA_BASE)>> 9)),VIDEO_VAL);
	
	outb_p(15, VIDEO_SEL);
	outb_p((unsigned char)(0xff & ((vga.pos- VGA_BASE)>> 1)),VIDEO_VAL);
}

static inline void get_curson(void)
{
	outb(14, VIDEO_SEL);
	vga.pos=inb(VIDEO_VAL)<<9;
		
	outb(15, VIDEO_SEL);
	vga.pos=vga.pos|(inb(VIDEO_VAL)<<1);	
}

static inline void set_orgin(void)
{
	cli ();
	// 首先选择显示控制数据寄存器r12，然后写入卷屏起始地址高字节。向右移动9 位，表示向右移动
	// 8 位，再除以2(2 字节代表屏幕上1 字符)。是相对于默认显示内存操作的。
		outb_p (12, VIDEO_SEL);
		outb_p ((unsigned char)(0xff & ((vga.origin - VGA_BASE) >> 9)),VIDEO_VAL);
	// 再选择显示控制数据寄存器r13，然后写入卷屏起始地址底字节。向右移动1 位表示除以2。
		outb_p (13, VIDEO_SEL);
		outb_p ((unsigned char)(0xff & ((vga.origin - VGA_BASE) >> 1)),VIDEO_VAL);	
	sti ();
}
//向上卷动一行（屏幕窗口向下移动）。
static void scrup (void)
{
	vga.origin += vga.cloumnNum*2;
	vga.end	+=	vga.cloumnNum*2;

	if(vga.end>=VGA_END){//超出显存
		memcpy((void *)VGA_BASE,(void *)vga.origin,vga.size-vga.cloumnNum*2);//复制最后一块屏
		vga.pos = vga.pos-vga.origin+VGA_BASE;
		vga.origin = VGA_BASE;
		vga.end = vga.origin+vga.size;
	}
	memset((void *)(vga.end-vga.cloumnNum*2),0,vga.cloumnNum*2);//清空新行
	set_orgin();
}

static inline void insert_line(void)
{
	
}

void vga_init()
{
	vga.mode=MODE_TEXT;	//set mode
	vga.cloumnNum=CLOUMN_NUM;
	vga.rowNum=ROW_NUM;
	vga.size=ROW_NUM*CLOUMN_NUM*2;
	vga.pos=VGA_BASE;		//光标
	vga.origin=VGA_BASE;	//显存起始
	vga.end=vga.origin+vga.size;//显存尾端
	vga.font=BLK_GRE;
}


