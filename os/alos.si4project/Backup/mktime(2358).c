#include<time.h>
#include<sys/types.h>
#include<asm/io.h>
#include<linux/kernel.h>
struct
{
	//addr:0
	u8	sec;
	u8	alarm_sec;
	u8	min;
	u8 	alarm_min;
	u8	hour;
	u8	alarm_hour;
	u8	wday;//week day
	u8	mday;//month day
	u8	mon;
	u8	year;
	u8	stateA;
	u8	stateB;
	u8	stateC;
	u8	stateD;//0:�����Ч 80:�����Ч
	u8	diagnose;//���
	u8	shut_stat;//�ػ�״̬
	
	//addr:0x10
	u8	floppy;
	u8	reserve0;
	u8	hd;//high 4 bit:c driver low 4 bits:d driver  0--�� f--�û��� ����ϵͳ��
	u8	reserve1;
	u8	dev_stat;
	u16	mem;
	u16	ext_mem;
	u8	type_c;	//c������
	u8	type_d;	//d������
	u16	reserve2;

	//addr:0x1d
	u16	cyl_c;	// ������
	u8	head_c;	// ��ͷ��
	u16	wpcom_c;// дǰԤ���������
	u16	lzone_c;//��ͷ��½�������
	u8	sect_c;	//ÿ�ŵ�������
	
	u16	cyl_d;	// ������
	u8	head_d;	// ��ͷ��
	u16	wpcom_d;// дǰԤ���������
	u16	lzone_d;//��ͷ��½�������
	u8	sect_d;	//ÿ�ŵ�������
	
	//addr:0x2d
	u8	reserve3;
	u16	check;	//У���  10~2D�ͣ�2EΪ��λ
	u16	extend;	//��չ�ڴ�
	u8	century;//����
	u8	flag;
	u8	res[12];
} coms;

static inline unsigned char CMOS_READ(unsigned char addr)
{
	outb_p(addr,0x70);
	return inb_p(0x71);
}

void coms_init(void)
{
	//addr:0
	coms.sec=CMOS_READ(0);
	coms.alarm_sec=CMOS_READ(1);
	coms.min=CMOS_READ(2);
	coms.alarm_min=CMOS_READ(3);
	coms.hour=CMOS_READ(4);
	coms.alarm_hour=CMOS_READ(5);
	coms.wday=CMOS_READ(6);//week day
	coms.mday=CMOS_READ(7);//month day
	coms.mon=CMOS_READ(8);
	coms.year=CMOS_READ(9);
	coms.stateA=CMOS_READ(0xa);
	coms.stateB=CMOS_READ(0xb);
	coms.stateC=CMOS_READ(0xc);
	coms.stateD=CMOS_READ(0xd);//0:�����Ч 80:�����Ч
	coms.diagnose=CMOS_READ(0xe);//���
	coms.shut_stat=CMOS_READ(0xf);//�ػ�״̬
	
	//addr:0x10
	coms.floppy=CMOS_READ(0x10);
	//coms.reserve0=CMOS_READ(0x11);
	coms.hd=CMOS_READ(0x12);//high 4 bit:c driver low 4 bits:d driver  0--�� f--�û��� ����ϵͳ��
	coms.reserve1=CMOS_READ(0x13);
	coms.dev_stat=CMOS_READ(0x14);
	coms.mem=CMOS_READ(0x15);
	coms.ext_mem=CMOS_READ(0x17);
	coms.type_c=CMOS_READ(0x19);	//c������
	coms.type_d=CMOS_READ(0x1a);	//d������
	//coms.reserve2=CMOS_READ(0x1b);

	//addr:0x1d
	coms.cyl_c=CMOS_READ(0x1d);	// ������
	coms.head_c=CMOS_READ(0x1f);	// ��ͷ��
	coms.wpcom_c=CMOS_READ(0x20);// дǰԤ���������
	coms.lzone_c=CMOS_READ(0x22);//��ͷ��½�������
	coms.sect_c=CMOS_READ(0x24);	//ÿ�ŵ�������
	
	coms.cyl_d=CMOS_READ(0x25);	// ������
	coms.head_d=CMOS_READ(0x27);	// ��ͷ��
	coms.wpcom_d=CMOS_READ(0x28);// дǰԤ���������
	coms.lzone_d=CMOS_READ(0x2a);//��ͷ��½�������
	coms.sect_d=CMOS_READ(0x2c);	//ÿ�ŵ�������
	
	//addr:0x2d
	//coms.reserve3=CMOS_READ(0x2d);
	coms.check=CMOS_READ(0x2e);	//У���  10~2D�ͣ�2EΪ��λ
	coms.extend=CMOS_READ(0x30);	//��չ�ڴ�
	coms.century=CMOS_READ(0x32);//����
	coms.flag=CMOS_READ(0x33);
	//coms.res[12];
}


// ��BCD ��ת�������֡�
#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)
// ���ӳ���ȡCMOS ʱ�ӣ������ÿ���ʱ�� startup_time(Ϊ��1970-1-1-0 ʱ�𵽿���ʱ������)��
void time_init(void)
{
	struct tm time;

	time.tm_sec = coms.sec;
	time.tm_min = coms.min;
	time.tm_hour = coms.hour;
	time.tm_mday = coms.mday;
	time.tm_mon = coms.mon;
	time.tm_year = coms.year;
	

	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_mday);
	BCD_TO_BIN(time.tm_mon);
	BCD_TO_BIN(time.tm_year);
	printk("sec:%d min:%d hour:%d mday:%d mon:%d year:%d\n",time.tm_sec,time.tm_sec,\
				time.tm_sec,time.tm_mday,time.tm_mon,time.tm_year);
	while(1);
	time.tm_mon--;
	//startup_time = kernel_mktime(&time);
}

#if 0
// �ú��������1970 ��1 ��1 ��0 ʱ�𵽿������վ�������������Ϊ����ʱ�䡣
long
kernel_mktime (struct tm *tm)
{
  long res;
  int year;

  year = tm->tm_year - 70;	// ��70 �굽���ھ���������(2 λ��ʾ��ʽ)��
  // ��˻���2000 �����⡣
  /* magic offsets (y+1) needed to get leapyears right. */
  /* Ϊ�˻����ȷ����������������Ҫ����һ��ħ��ƫֵ(y+1) */
  res = YEAR * year + DAY * ((year + 1) / 4);	// ��Щ�꾭��������ʱ�� + ÿ������ʱ��1 ��
  res += month[tm->tm_mon];	// ������ʱ�䣬�ڼ��ϵ��굽����ʱ��������
  /* and (y+2) here. If it wasn't a leap-year, we have to adjust */
  /* �Լ�(y+2)�����(y+2)�������꣬��ô���Ǿͱ�����е���(��ȥһ�������ʱ��)�� */
  if (tm->tm_mon > 1 && ((year + 2) % 4))
    res -= DAY;
  res += DAY * (tm->tm_mday - 1);	// �ټ��ϱ��¹�ȥ������������ʱ�䡣
  res += HOUR * tm->tm_hour;	// �ټ��ϵ����ȥ��Сʱ��������ʱ�䡣
  res += MINUTE * tm->tm_min;	// �ټ���1 Сʱ�ڹ�ȥ�ķ�����������ʱ�䡣
  res += tm->tm_sec;		// �ټ���1 �������ѹ���������
  return res;			// �����ڴ�1970 ����������������ʱ�䡣
}
#endif



