#include<drivers/vga.h>	//VGA相关参数
#include<asm/io.h>	//端口输入输出	
#include<ctype.h>	//判断字符类型

#include<linux/config.h>	
#include<asm/system.h>


static vga_t vga;
static inline  void set_curson(void);
static inline  void get_curson(void);

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
		//清屏
		if(vga.pos==0){
			i=vga.cloumnNum*vga.rowNum-1;
			vga.pos+=2;
			while(i--)vga_write(' ');
		}

		//输出一个可显示字符
		*(char *)(vga.pos+vga.base)=charactor;
		*(char *)(vga.pos+vga.base+1)=vga.font;
		vga.pos=(vga.pos+2)%(vga.cloumnNum*vga.rowNum*2);
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
			case '\r':vga.pos=vga.pos-vga.pos%(vga.cloumnNum*2);break;
			default :puts("\nctrl char not support now!!\n");
		}
		set_curson();
		return ;
	}	
	puts("\nerror charactor!!\n");
}


static inline void set_curson(void)
{

	outb(14, VIDEO_SEL);
	outb((unsigned char)(0xff & (vga.pos>> 9)),VIDEO_VAL);
	
	outb(15, VIDEO_SEL);
	outb((unsigned char)(0xff & (vga.pos>> 1)),VIDEO_VAL);
}

static inline void get_curson(void)
{
	cli();
	outb(14, VIDEO_SEL);
	vga.pos=inb(VIDEO_VAL)<<9;
		
	outb(15, VIDEO_SEL);
	vga.pos=vga.pos|(inb(VIDEO_VAL)<<1);	
	sti ();
}

static inline void insert_line(void)
{
	
}

void vga_init()
{
	vga.mode=MODE_TEXT;	//set mode
	vga.cloumnNum=CLOUMN_NUM;
	vga.rowNum=ROW_NUM;
	vga.base=VGA_PHY_BASE+VM_START;
	vga.pos=0;
	vga.font=BLK_GRE;
}


