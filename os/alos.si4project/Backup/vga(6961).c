#include<drivers/vga.h>	//VGA��ز���
#include<asm/io.h>	//�˿��������	
#include<ctype.h>	//�ж��ַ�����

#include<linux/config.h>	
#include<asm/system.h>
#include<string.h>

static struct{
	u8 mode;	//ģʽ(no used)
	u32 pos;	//���
	u32 origin;	//�Դ���ʼ(ָ������ʾ���ڴ�)
	u32 end;	//�Դ�β��
	u8 rowNum;	//��
	u8 cloumnNum;//��
	u32 size;	//һ���ڴ�
	u8 font;	//����
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
		if(vga.pos>=vga.end)//�����������Ļ
			scrup();
		//���һ������ʾ�ַ�
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
			case '\t':i=4;	//Ĭ��4���ո�
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
	// ����ѡ����ʾ�������ݼĴ���r12��Ȼ��д�������ʼ��ַ���ֽڡ������ƶ�9 λ����ʾ�����ƶ�
	// 8 λ���ٳ���2(2 �ֽڴ�����Ļ��1 �ַ�)���������Ĭ����ʾ�ڴ�����ġ�
		outb_p (12, VIDEO_SEL);
		outb_p ((unsigned char)(0xff & ((vga.origin - VGA_BASE) >> 9)),VIDEO_VAL);
	// ��ѡ����ʾ�������ݼĴ���r13��Ȼ��д�������ʼ��ַ���ֽڡ������ƶ�1 λ��ʾ����2��
		outb_p (13, VIDEO_SEL);
		outb_p ((unsigned char)(0xff & ((vga.origin - VGA_BASE) >> 1)),VIDEO_VAL);	
	sti ();
}
//���Ͼ�һ�У���Ļ���������ƶ�����
static void scrup (void)
{
	vga.origin += vga.cloumnNum*2;
	vga.end	+=	vga.cloumnNum*2;

	if(vga.end>=VGA_END){//�����Դ�
		memcpy((void *)VGA_BASE,(void *)vga.origin,vga.size-vga.cloumnNum*2);//�������һ����
		vga.pos = vga.pos-vga.origin+VGA_BASE;
		vga.origin = VGA_BASE;
		vga.end = vga.origin+vga.size;
	}
	memset((void *)(vga.end-vga.cloumnNum*2),0,vga.cloumnNum*2);//�������
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
	vga.pos=VGA_BASE;		//���
	vga.origin=VGA_BASE;	//�Դ���ʼ
	vga.end=vga.origin+vga.size;//�Դ�β��
	vga.font=BLK_GRE;
}


