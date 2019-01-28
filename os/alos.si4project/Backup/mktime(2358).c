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
	u8	stateD;//0:电池无效 80:电池有效
	u8	diagnose;//诊断
	u8	shut_stat;//关机状态
	
	//addr:0x10
	u8	floppy;
	u8	reserve0;
	u8	hd;//high 4 bit:c driver low 4 bits:d driver  0--无 f--用户盘 其余系统盘
	u8	reserve1;
	u8	dev_stat;
	u16	mem;
	u16	ext_mem;
	u8	type_c;	//c盘类型
	u8	type_d;	//d盘类型
	u16	reserve2;

	//addr:0x1d
	u16	cyl_c;	// 柱面数
	u8	head_c;	// 磁头数
	u16	wpcom_c;// 写前预补偿柱面号
	u16	lzone_c;//磁头着陆区柱面号
	u8	sect_c;	//每磁道扇区数
	
	u16	cyl_d;	// 柱面数
	u8	head_d;	// 磁头数
	u16	wpcom_d;// 写前预补偿柱面号
	u16	lzone_d;//磁头着陆区柱面号
	u8	sect_d;	//每磁道扇区数
	
	//addr:0x2d
	u8	reserve3;
	u16	check;	//校验和  10~2D和，2E为高位
	u16	extend;	//扩展内存
	u8	century;//世纪
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
	coms.stateD=CMOS_READ(0xd);//0:电池无效 80:电池有效
	coms.diagnose=CMOS_READ(0xe);//诊断
	coms.shut_stat=CMOS_READ(0xf);//关机状态
	
	//addr:0x10
	coms.floppy=CMOS_READ(0x10);
	//coms.reserve0=CMOS_READ(0x11);
	coms.hd=CMOS_READ(0x12);//high 4 bit:c driver low 4 bits:d driver  0--无 f--用户盘 其余系统盘
	coms.reserve1=CMOS_READ(0x13);
	coms.dev_stat=CMOS_READ(0x14);
	coms.mem=CMOS_READ(0x15);
	coms.ext_mem=CMOS_READ(0x17);
	coms.type_c=CMOS_READ(0x19);	//c盘类型
	coms.type_d=CMOS_READ(0x1a);	//d盘类型
	//coms.reserve2=CMOS_READ(0x1b);

	//addr:0x1d
	coms.cyl_c=CMOS_READ(0x1d);	// 柱面数
	coms.head_c=CMOS_READ(0x1f);	// 磁头数
	coms.wpcom_c=CMOS_READ(0x20);// 写前预补偿柱面号
	coms.lzone_c=CMOS_READ(0x22);//磁头着陆区柱面号
	coms.sect_c=CMOS_READ(0x24);	//每磁道扇区数
	
	coms.cyl_d=CMOS_READ(0x25);	// 柱面数
	coms.head_d=CMOS_READ(0x27);	// 磁头数
	coms.wpcom_d=CMOS_READ(0x28);// 写前预补偿柱面号
	coms.lzone_d=CMOS_READ(0x2a);//磁头着陆区柱面号
	coms.sect_d=CMOS_READ(0x2c);	//每磁道扇区数
	
	//addr:0x2d
	//coms.reserve3=CMOS_READ(0x2d);
	coms.check=CMOS_READ(0x2e);	//校验和  10~2D和，2E为高位
	coms.extend=CMOS_READ(0x30);	//扩展内存
	coms.century=CMOS_READ(0x32);//世纪
	coms.flag=CMOS_READ(0x33);
	//coms.res[12];
}


// 将BCD 码转换成数字。
#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)
// 该子程序取CMOS 时钟，并设置开机时间 startup_time(为从1970-1-1-0 时起到开机时的秒数)。
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
// 该函数计算从1970 年1 月1 日0 时起到开机当日经过的秒数，作为开机时间。
long
kernel_mktime (struct tm *tm)
{
  long res;
  int year;

  year = tm->tm_year - 70;	// 从70 年到现在经过的年数(2 位表示方式)，
  // 因此会有2000 年问题。
  /* magic offsets (y+1) needed to get leapyears right. */
  /* 为了获得正确的闰年数，这里需要这样一个魔幻偏值(y+1) */
  res = YEAR * year + DAY * ((year + 1) / 4);	// 这些年经过的秒数时间 + 每个闰年时多1 天
  res += month[tm->tm_mon];	// 的秒数时间，在加上当年到当月时的秒数。
  /* and (y+2) here. If it wasn't a leap-year, we have to adjust */
  /* 以及(y+2)。如果(y+2)不是闰年，那么我们就必须进行调整(减去一天的秒数时间)。 */
  if (tm->tm_mon > 1 && ((year + 2) % 4))
    res -= DAY;
  res += DAY * (tm->tm_mday - 1);	// 再加上本月过去的天数的秒数时间。
  res += HOUR * tm->tm_hour;	// 再加上当天过去的小时数的秒数时间。
  res += MINUTE * tm->tm_min;	// 再加上1 小时内过去的分钟数的秒数时间。
  res += tm->tm_sec;		// 再加上1 分钟内已过的秒数。
  return res;			// 即等于从1970 年以来经过的秒数时间。
}
#endif



