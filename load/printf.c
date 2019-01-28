#include<vga.h>

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

static inline void set_curson(void)
{
	outb_p(14, VIDEO_SEL);
	outb_p((unsigned char)(0xff & ((vga.pos- VGA_BASE)>> 9)),VIDEO_VAL);
	
	outb_p(15, VIDEO_SEL);
	outb_p((unsigned char)(0xff & ((vga.pos- VGA_BASE)>> 1)),VIDEO_VAL);
}

void vga_write(char charactor)
{
	int i;
	if(isprint(charactor))
	{
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
					while(i--)vga_write(' ');vga_write('\r');break;
			case '\t':i=4;	//默认4个空格
					while(i--)vga_write(' ');break;
			case '\r':vga.pos -= (vga.pos-VGA_BASE)%(vga.cloumnNum*2);break;
			default :vga_write('*');
		}
		set_curson();
		return ;
	}	
	vga_write('*');
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
	vga.font=BLK_BLU_L;
}


static char buf[1024];
extern int vsprintf(char * buf, const char * fmt, va_list args);

int printf(const char *fmt, ...)
{
	va_list args;
	int i;
	char *c=buf;

	va_start(args, fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);

	while(i--)
		vga_write(*c++);
	
	return i;
}

