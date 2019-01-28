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
} cmos;

static inline unsigned char CMOS_READ(unsigned char addr)
{
	outb_p(addr,0x70);
	return inb_p(0x71);
}

void coms_init(void)
{
	//addr:0
	cmos.sec=CMOS_READ(0);
	cmos.alarm_sec=CMOS_READ(1);
	cmos.min=CMOS_READ(2);
	cmos.alarm_min=CMOS_READ(3);
	cmos.hour=CMOS_READ(4);
	cmos.alarm_hour=CMOS_READ(5);
	cmos.wday=CMOS_READ(6);//week day
	cmos.mday=CMOS_READ(7);//month day
	cmos.mon=CMOS_READ(8);
	cmos.year=CMOS_READ(9);
	cmos.stateA=CMOS_READ(0xa);
	cmos.stateB=CMOS_READ(0xb);
	cmos.stateC=CMOS_READ(0xc);
	cmos.stateD=CMOS_READ(0xd);//0:�����Ч 80:�����Ч
	cmos.diagnose=CMOS_READ(0xe);//���
	cmos.shut_stat=CMOS_READ(0xf);//�ػ�״̬
	
	//addr:0x10
	cmos.floppy=CMOS_READ(0x10);
	//cmos.reserve0=CMOS_READ(0x11);
	cmos.hd=CMOS_READ(0x12);//high 4 bit:c driver low 4 bits:d driver  0--�� f--�û��� ����ϵͳ��
	cmos.reserve1=CMOS_READ(0x13);
	cmos.dev_stat=CMOS_READ(0x14);
	cmos.mem=CMOS_READ(0x15);
	cmos.ext_mem=CMOS_READ(0x17);
	cmos.type_c=CMOS_READ(0x19);	//c������
	cmos.type_d=CMOS_READ(0x1a);	//d������
	//cmos.reserve2=CMOS_READ(0x1b);

	//addr:0x1d
	cmos.cyl_c=CMOS_READ(0x1d);	// ������
	cmos.head_c=CMOS_READ(0x1f);	// ��ͷ��
	cmos.wpcom_c=CMOS_READ(0x20);// дǰԤ���������
	cmos.lzone_c=CMOS_READ(0x22);//��ͷ��½�������
	cmos.sect_c=CMOS_READ(0x24);	//ÿ�ŵ�������
	
	cmos.cyl_d=CMOS_READ(0x25);	// ������
	cmos.head_d=CMOS_READ(0x27);	// ��ͷ��
	cmos.wpcom_d=CMOS_READ(0x28);// дǰԤ���������
	cmos.lzone_d=CMOS_READ(0x2a);//��ͷ��½�������
	cmos.sect_d=CMOS_READ(0x2c);	//ÿ�ŵ�������
	
	//addr:0x2d
	//cmos.reserve3=CMOS_READ(0x2d);
	cmos.check=CMOS_READ(0x2e);	//У���  10~2D�ͣ�2EΪ��λ
	cmos.extend=CMOS_READ(0x30);	//��չ�ڴ�
	cmos.century=CMOS_READ(0x32);//����
	cmos.flag=CMOS_READ(0x33);
	//cmos.res[12];
}


// ��BCD ��ת�������֡�
#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)
// ���ӳ���ȡCMOS ʱ�ӣ������ÿ���ʱ�� startup_time(Ϊ��1970-1-1-0 ʱ�𵽿���ʱ������)��
void time_init(void)
{
	struct tm time;

	time.tm_sec = cmos.sec;
	time.tm_min = cmos.min;
	time.tm_hour = cmos.hour;
	time.tm_mday = cmos.mday;
	time.tm_mon = cmos.mon;
	time.tm_year = cmos.year;
	

	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_min);
	BCD_TO_BIN(time.tm_hour);
	BCD_TO_BIN(time.tm_mday);
	BCD_TO_BIN(time.tm_mon);
	BCD_TO_BIN(time.tm_year);
	printk("sec:%d min:%d hour:%d mday:%d mon:%d year:%d\n",time.tm_sec,time.tm_min,\
				time.tm_hour,time.tm_mday,time.tm_mon,time.tm_year);
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



